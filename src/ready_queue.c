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
      run_PCB_FCFS(peek_front(queue), queue);
    }
  }

  // round robin
  else if (strcmp(policy, "RR") == 0)
  {
    return -1;
  }

  // shortest job first
  else if (strcmp(policy, "SJF") == 0)
  {
    return -1;
  }
  else
    return -1;

  return 0;
}

int run_PCB_FCFS(struct PCB pcb, struct PCBreadyqueue *queue)
{
  int curr_instr_index = pcb.current_instruction;
  while (curr_instr_index < pcb.num_instructions)
  {
    // start at the currently running instruction
    char identifier[100];
    sprintf(identifier, "%d-%d", pcb.pid, curr_instr_index);

    char *curr_instruction = mem_get_command_value(pcb.script_location_start, curr_instr_index, identifier);
    int errorCode = parseInput(curr_instruction);

    if (errorCode == -1)
    {
      return -1;
    }

    curr_instr_index++;
  }

  if (curr_instr_index == pcb.num_instructions)
  {
    pcb.current_instruction = curr_instr_index;
    dequeue(queue);
  }

  return 0;
}