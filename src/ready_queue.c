#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   //new included library for chdir
#include <sys/stat.h> //new included library for stat
#include "ready_queue.h"
#include "shellmemory.h"
#include "shell.h"

struct PCBreadyqueue *create_ready_queue(int capacity)
{
  struct PCBreadyqueue *queue = (struct PCBreadyqueue *)malloc(sizeof(struct PCBreadyqueue));
  queue->capacity = capacity;
  queue->queue_start = 0;
  queue->size = 0;
  queue->queue_end = capacity - 1;
  queue->queue_array = (struct PCB *)malloc(queue->capacity * sizeof(struct PCB));

  return queue;
}

// add a pcb to back of queue
int enqueue(struct PCBreadyqueue *queue, struct PCB new_pcb)
{
  if (is_full(queue))
    return -1;
  queue->queue_end = (queue->queue_end + 1) % queue->capacity;
  queue->queue_array[queue->queue_end] = new_pcb;
  queue->size = queue->size + 1;

  return 0;
}

// remove pcb from front of queue
struct PCB dequeue(struct PCBreadyqueue *queue)
{
  if (is_empty(queue))
    printf("Queue is empty");
  struct PCB pcb_to_remove = queue->queue_array[queue->queue_start];
  queue->queue_start = (queue->queue_start + 1) % queue->capacity;
  queue->size = queue->size - 1;

  return pcb_to_remove;
}

// look at first element in queue without removing
struct PCB peek_front(struct PCBreadyqueue *queue)
{
  if (is_empty(queue))
    printf("Queue is empty");
  return queue->queue_array[queue->queue_start];
}

// look at last element in queue without removing
struct PCB peek_end(struct PCBreadyqueue *queue)
{
  if (is_empty(queue))
    printf("Queue is empty");
  return queue->queue_array[queue->queue_end];
}

// check if queue is empty
int is_empty(struct PCBreadyqueue *queue)
{
  return (queue->size == 0);
}

// check if queue is full
int is_full(struct PCBreadyqueue *queue)
{
  return (queue->size == queue->capacity);
}

// remove a specific PCB given its index in the queue array, not necessarily the one at the front of the queue
int remove_specific_PCB(struct PCBreadyqueue *queue, int index)
{
  if (is_empty(queue))
  {
    return -1;
  }
  queue->queue_start = (index + 1) % (queue->capacity);
  queue->size = queue->size - 1;
  return 0;
}

// for sorting the array of PCBs
// Code Inspiration: https://stackoverflow.com/questions/8721189/how-to-sort-an-array-of-structs-in-c
int compare_instruction_length(const void *pcb_1, const void *pcb_2)
{
  struct PCB *p1 = (struct PCB *)pcb_1;
  struct PCB *p2 = (struct PCB *)pcb_2;
  if (p1->num_instructions < p2->num_instructions)
    return -1;
  else if (p1->num_instructions > p2->num_instructions)
    return +1;
  else
    return 0;
}

// run ready queue with either first come first serve (FCFS), round robin (RR), shortest job first (SJF)
int run_ready_queue(struct PCBreadyqueue *queue, char *policy)
{
  // make sure queue isn't empty
  if (is_empty(queue))
  {
    return -1;
  }

  // first come first serve
  // just dequeue from list and run in order
  if (strcmp(policy, "FCFS") == 0)
  {
    int i;
    while (!is_empty(queue))
    {
      struct PCB curr_pcb = peek_front(queue);
      run_PCB_FCFS(curr_pcb, queue);
      dequeue(queue);
      remove_script(curr_pcb);

      // printf("dequeued %d\n", curr_pcb.pid);

      // printf("shell memory is:\n");
      // show_memory();
    }
  }

  // round robin
  // each program runs 2 instructions each before being switched
  else if (strcmp(policy, "RR") == 0)
  {
    // start at the front of the queue, and add on size modulus size to get the next
    int curr_pcb_index = queue->queue_start;
    int terminated_count = 0; // keep track of the number of finished processes

    int active_PCBs[queue->size];
    for (int x = 0; x < queue->size; x++)
    {
      active_PCBs[x] = 1; // 1 for ative, 0 for inactive
    }

    while (terminated_count < (queue->size))
    {
      struct PCB *curr_pcb = &(queue->queue_array[curr_pcb_index]);
      if (curr_pcb->current_instruction < curr_pcb->num_instructions)
      {
        // printf("while again, curr is %d\n", curr_pcb.current_instruction);
        run_PCB_RR(curr_pcb); // run another 2 instructions if not finished yet
      }
      else if (active_PCBs[curr_pcb->pid] == 1)
      {
        // once finished running remove the script but for the sake of simplicity
        // wait until everything is done to dequeue
        terminated_count++;
        remove_script(*curr_pcb);
        active_PCBs[curr_pcb->pid] = 0;
      }
      curr_pcb_index = (curr_pcb_index + 1) % (queue->capacity);
    }

    // now dequeue everything
    for (int i = 0; i < terminated_count; i++)
    {
      dequeue(queue);
    }
  }

  // shortest job first
  // program with fewest lines goes first
  else if (strcmp(policy, "SJF") == 0)
  {
    // reorder queue basde on number of instructions
    // using bubblesort for the sake of simplicity, plus we only need to deal with 3 processes at most

    // struct PCB* new_queue_array = (struct PCB *)malloc(queue->size * sizeof(struct PCB));

    queue->queue_start = 0;
    queue->queue_end = queue->size - 1;

    int i, j;
    for (i = 0; i < queue->size - 1; i++)
    {
      for (j = 0; j < queue->size - i - 1; j++)
      {
        if ((queue->queue_array[j]).num_instructions > (queue->queue_array[j + 1]).num_instructions)
        {
          struct PCB temp_PCB = queue->queue_array[j];
          queue->queue_array[j] = queue->queue_array[j + 1];
          queue->queue_array[j + 1] = temp_PCB;
        }
      }
    }
    while (!is_empty(queue))
    {
      struct PCB curr_pcb = peek_front(queue);
      run_PCB_FCFS(curr_pcb, queue);
      dequeue(queue);
      remove_script(curr_pcb);

      // printf("shell memory is:\n");
      // show_memory();
    }
  }
  // SJF with aging
  // this is going to be pretty inefficient
  else if (strcmp(policy, "AGING") == 0)
  {
    int terminated_count = 0; // keep track of the number of finished processes

    int active_PCBs[queue->size];
    for (int x = 0; x < queue->size; x++)
    {
      active_PCBs[x] = 1; // 1 for ative, 0 for inactive
    }

    // start off with SJF and sort array
    bubble_sort(queue);

    int curr_pcb_index = queue->queue_start;

    // reassess queueu every 1 instruction
    while (terminated_count < (queue->size))
    {
      struct PCB *curr_pcb = &(queue->queue_array[queue->queue_start]); // front of queue
      if (curr_pcb->current_instruction < curr_pcb->num_instructions)
      {
        run_PCB_AGING(curr_pcb); // execute 1 instruction in current process

        // age all jobs in ready queue aside from head by decreasing job length score
        for (int i = 0; i < queue->capacity; i++)
        {
          struct PCB *to_age_pcb = &(queue->queue_array[i]);
          if ((i != queue->queue_start) && (to_age_pcb->job_length_score > 0))
          {
            to_age_pcb->job_length_score = to_age_pcb->job_length_score - 1;
          }
        }

        // resort queue so shortest job length in front
        bubble_sort(queue);
      }
      else if (active_PCBs[curr_pcb->pid] == 1)
      {
        // once finished running remove the script but for the sake of simplicity
        // wait until everything is done to dequeue
        terminated_count++;
        remove_script(*curr_pcb);
        active_PCBs[curr_pcb->pid] = 0;
      }
    }

    // now dequeue everything
    for (int i = 0; i < terminated_count; i++)
    {
      dequeue(queue);
    }
    
  }
  else
    return -1;

  return 0;
}

int bubble_sort(struct PCBreadyqueue *queue)
{
  queue->queue_start = 0;
  queue->queue_end = queue->size - 1;

  int i, j;
  for (i = 0; i < queue->size - 1; i++)
  {
    for (j = 0; j < queue->size - i - 1; j++)
    {
      if ((queue->queue_array[j]).job_length_score > (queue->queue_array[j + 1]).job_length_score)
      {
        struct PCB temp_PCB = queue->queue_array[j];
        queue->queue_array[j] = queue->queue_array[j + 1];
        queue->queue_array[j + 1] = temp_PCB;
      }
    }
  }
  return 0;
}

int run_PCB_FCFS(struct PCB pcb, struct PCBreadyqueue *queue)
{
  // printf("memory is:\n");
  // show_memory();
  int curr_instr_index = pcb.current_instruction;
  while (curr_instr_index < pcb.num_instructions)
  {
    // start at the currently running instruction
    char identifier[100];
    sprintf(identifier, "%d-%d", pcb.pid, curr_instr_index);

    char *curr_instruction = mem_get_command_value(pcb.script_location_start, curr_instr_index, identifier);
    // printf("curr instr is %s\n", curr_instruction);
    // printf("calling\n");
    int errorCode = parseInput(curr_instruction);

    if (errorCode == -1)
    {
      return -1;
    }

    curr_instr_index++;
  }

  pcb.current_instruction = curr_instr_index;

  // if (curr_instr_index == pcb.num_instructions)
  // {
  //   pcb.current_instruction = curr_instr_index;
  //   struct PCB removed_pcb = dequeue(queue);
  //   remove_script(removed_pcb);
  // }

  // printf("shell memory is:\n");
  // show_memory();

  return 0;
}

// run 2 instructions at a time
// ugh just return the new instruction for now. I'll fix it later
int run_PCB_RR(struct PCB *pcb)
{
  // printf("memory is:\n");
  // show_memory();
  int curr_instr_index = pcb->current_instruction;
  int instruction_count = 0;
  while ((curr_instr_index < pcb->num_instructions) && (instruction_count < 2))
  {
    // start at the currently running instruction
    char identifier[100];
    sprintf(identifier, "%d-%d", pcb->pid, curr_instr_index);

    char *curr_instruction = mem_get_command_value(pcb->script_location_start, curr_instr_index, identifier);
    // printf("curr instr is %s\n", curr_instruction);
    int errorCode = parseInput(curr_instruction);

    if (errorCode == -1)
    {
      return -1;
    }

    curr_instr_index++;
    instruction_count++;

    // printf("index and instrcount: %d and %d\n", curr_instr_index, instruction_count);
  }

  pcb->current_instruction = curr_instr_index;
  // printf("new curr instr is %d\n", pcb.current_instruction);

  return 0;
}

// run just one instruction at a time
int run_PCB_AGING(struct PCB *pcb)
{
  if (pcb->current_instruction < pcb->num_instructions)
  {
    // start at the currently running instruction
    char identifier[100];
    sprintf(identifier, "%d-%d", pcb->pid, pcb->current_instruction);

    char *curr_instruction = mem_get_command_value(pcb->script_location_start, pcb->current_instruction, identifier);
    // printf("curr instr is %s\n", curr_instruction);
    int errorCode = parseInput(curr_instruction);

    if (errorCode == -1)
    {
      return -1;
    }

    pcb->current_instruction = pcb->current_instruction + 1;
  }
  return 0;
}

// remove single script
int remove_script(struct PCB pcb)
{
  mem_clean_out_block(pcb.script_location_start, pcb.num_instructions);
  return 0;
}