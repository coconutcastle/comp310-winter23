#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "pcb.h"
#include "kernel.h"
#include "shell.h"
#include "shellmemory.h"
#include "interpreter.h"
#include "ready_queue.h"

struct QueueNode *head = NULL;
struct QueueNode *all_nodes[3] = {NULL, NULL, NULL};

void ready_queue_destory()
{
  if (!head)
    return;
  struct QueueNode *cur = head;
  struct QueueNode *tmp;
  while (cur->next != NULL)
  {
    tmp = cur->next;
    free(cur);
    cur = tmp;
  }
  free(cur);
}

void ready_queue_add_to_tail(struct QueueNode *node)
{
  int does_contain = -1;
  int node_count = 0;
  int first_null_index = -1;
  for (int i = 0; i < 3; i++)
  {
    if (all_nodes[i] != NULL)
    {
      if (all_nodes[i]->pcb->pid == node->pcb->pid)
      {
        does_contain = 1;
      }
      node_count++;
    }
    else
    {
      if (first_null_index == -1)
      {
        first_null_index = i;
      }
    }
  }
  // for (int i = 0; i < 3; i++) {
  //   if (all_nodes[i] != NULL) {
  //     printf("%d %s\n", i, all_nodes[i]->pcb->progname);
  //   }
  // }

  // if not already in list and there is space, add
  if (does_contain == -1 && node_count < 3)
  {
    all_nodes[first_null_index] = node;
  }
  if (!head)
  {
    head = node;
    head->next = NULL;
  }
  else
  {
    struct QueueNode *cur = head;
    while (cur->next != NULL)
      cur = cur->next;
    cur->next = node;
    cur->next->next = NULL;
  }
}

// void age_all_nodes(struct PCB *curr_pcb, int curr_frame)
// {
//   // printf("%s\n", "now aging");
//   for (int i = 0; i < 3; i++)
//   {
//     if (all_nodes[i] != NULL)
//     {
//       struct QueueNode *curr_node = all_nodes[i];
//       for (int j = 0; j < curr_node->pcb->page_table_size; j++)
//       {
//         if ((curr_node->pcb->pid != curr_pcb->pid) || (curr_node->pcb->page_table[i].frame != curr_frame))
//         {
//           if ((curr_node->pcb->page_table[j].last_used != -1))
//           {
//             // printf("aging up %s %d, is %d\n", pcb->progname, pcb->page_table[f].frame, pcb->page_table[f].last_used);
//             curr_node->pcb->page_table[j].last_used = curr_node->pcb->page_table[j].last_used + 1;
//           }
//         }
//       }
//     }
//   }
//   // printf("%s\n", "done aging");
// }

void age_all_nodes(struct PCB *curr_pcb, int curr_frame)
{
  // printf("%s\n", "now aging");
  for (int i = 0; i < 3; i++)
  {
    if (all_nodes[i] != NULL)
    {
      struct QueueNode *curr_node = all_nodes[i];
      for (int j = 0; j < curr_node->pcb->page_table_size; j++)
      {
          if ((curr_node->pcb->page_table[j].last_used != -1))
          {
            // printf("aging up %s %d, is %d\n", pcb->progname, pcb->page_table[f].frame, pcb->page_table[f].last_used);
            curr_node->pcb->page_table[j].last_used = curr_node->pcb->page_table[j].last_used + 1;
          }
        
      }
    }
  }
  // printf("%s\n", "done aging");
}

void print_node_ages()
{
  for (int i = 0; i < 3; i++)
  {
    if (all_nodes[i] != NULL)
    {
      struct QueueNode *curr_node = all_nodes[i];
      for (int j = 0; j < curr_node->pcb->page_table_size; j++)
      {

        if ((curr_node->pcb->page_table[j].last_used != -1))
        {
          printf("program %s, frame %d, age %d\n", curr_node->pcb->progname, curr_node->pcb->page_table[j].frame, curr_node->pcb->page_table[j].last_used);
        }
      }
    }
  }
}

struct LRU_frame *find_lru()
{
  struct LRU_frame *lru = malloc(sizeof(struct LRU_frame));
  lru->pcb = NULL;
  lru->victimFrame = -1;

  int max_age = -1;

  for (int i = 0; i < 3; i++)
  {
    if (all_nodes[i] != NULL)
    {
      struct QueueNode *curr_node = all_nodes[i];
      // printf("looking at %s\n", curr_node->pcb->progname);
      for (int j = 0; j < curr_node->pcb->page_table_size; j++)
      {
        if (curr_node->pcb->page_table[j].last_used > max_age)
        {
          max_age = curr_node->pcb->page_table[j].last_used;
          lru->victimFrame = curr_node->pcb->page_table[j].frame;
          lru->pcb = curr_node->pcb;
          lru->page_index = j;
        }
      }
    }
  }
  return lru;
}

void ready_queue_add_to_head(struct QueueNode *node)
{
  if (!head)
  {
    head = node;
    head->next = NULL;
  }
  else
  {
    node->next = head;
    head = node;
  }
}

struct QueueNode *ready_queue_get_head()
{
  // if (head == NULL) {
  //   printf("%s\n", "head is null");
  // }
  return head;
}

void print_ready_queue()
{
  if (!head)
  {
    printf("%s\n", "ready queue is empty");
    return;
  }
  struct QueueNode *cur = head;
  printf("%s\n", "Ready queue: ");
  while (cur != NULL)
  {
    printf("\tPID: %d, length: %d, priority: %d, pc: %d\n", cur->pcb->pid, cur->pcb->num_lines, cur->pcb->priority, cur->pcb->program_counter);
    cur = cur->next;
  }
}

void terminate_process(struct QueueNode *node)
{
  // node should not be in the ready queue

  // get all pages associated with process, always in chunks of 3
  struct PTE *pte = node->pcb->page_table;

  for (int i = 0; i < node->pcb->page_table_size; i++)
  {
    if (pte[i].valid == 1)
    {
      mem_free_lines_between(pte[i].frame * 3, (pte[i].frame * 3) + 2);
    }
  }

  // printf("%s\n", "terminated");
  free(node);
}

bool is_ready_empty()
{
  return head == NULL;
}

struct QueueNode *ready_queue_pop_head()
{
  struct QueueNode *tmp = head;
  if (head != NULL)
    head = head->next;
  // printf("popped %d at %d\n", tmp->pcb->pid, tmp->pcb->program_counter);
  return tmp;
}

void ready_queue_decrement_job_length_score()
{
  struct QueueNode *cur;
  cur = head;
  while (cur != NULL)
  {
    if (cur->pcb->job_length_score > 0)
      cur->pcb->job_length_score--;
    cur = cur->next;
  }
}

void ready_queue_swap_with_next(struct QueueNode *toSwap)
{
  struct QueueNode *next;
  struct QueueNode *afterNext;
  struct QueueNode *cur = head;
  if (head == toSwap)
  {
    next = head->next;
    head->next = next->next;
    next->next = head;
    head = next;
  }
  while (cur != NULL && cur->next != toSwap)
    cur = cur->next;
  if (cur == NULL)
    return;
  next = cur->next->next;
  afterNext = next->next;
  // cur toSwap next afterNext
  cur->next = next;
  next->next = toSwap;
  toSwap->next = afterNext;
}

bool swap_needed(struct QueueNode *cur)
{
  struct QueueNode *next = cur->next;
  if (!next)
    return false;
  if (cur->pcb->priority && next->pcb->priority)
  {
    if (cur->pcb->job_length_score > next->pcb->job_length_score)
    {
      return true;
    }
    else
      return false;
  }
  else if (cur->pcb->priority && !next->pcb->priority)
    return false;
  else if (!cur->pcb->priority && next->pcb->priority)
    return true;
  else
  {
    if (cur->pcb->job_length_score > next->pcb->job_length_score)
      return true;
    else
      return false;
  }
}

void sort_ready_queue()
{
  if (head == NULL)
    return;
  // bubble sort
  struct QueueNode *cur = head;
  bool sorted = false;
  while (!sorted)
  {
    sorted = true;
    while (cur->next != NULL)
    {
      if (swap_needed(cur))
      {
        sorted = false;
        ready_queue_swap_with_next(cur);
      }
      else
      {
        cur = cur->next;
      }
    }
  }
}

struct QueueNode *ready_queue_pop_shortest_job()
{
  sort_ready_queue();
  struct QueueNode *node = ready_queue_pop_head();
  return node;
}

int ready_queue_get_shortest_job_score()
{
  struct QueueNode *cur = head;
  int shortest = MAX_INT;
  while (cur != NULL)
  {
    if (cur->pcb->job_length_score < shortest)
    {
      shortest = cur->pcb->job_length_score;
    }
    cur = cur->next;
  }
  return shortest;
}

void ready_queue_promote(int score)
{
  if (head->pcb->job_length_score == score)
    return;
  struct QueueNode *cur = head;
  struct QueueNode *next;
  while (cur->next != NULL)
  {
    if (cur->next->pcb->job_length_score == score)
      break;
    cur = cur->next;
  }
  if (cur->next == NULL || cur->next->pcb->job_length_score != score)
    return;
  next = cur->next;
  cur->next = cur->next->next;
  next->next = head;
  head = next;
}