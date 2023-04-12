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
struct QueueNode *all_nodes[3] = {NULL, NULL, NULL};    // gonna assume no more than 3 programs at once

void ready_queue_destory()
{
  if (!head)
    return;
  struct QueueNode *cur = head;
  struct QueueNode *tmp;
  while (cur->next != NULL)
  {
    tmp = cur->next;
    free(cur->pcb->progname);
    free(cur->pcb->filename);
    free(cur->pcb);
    free(cur);
    cur = tmp;
  }
  free(cur->pcb->progname);
  free(cur->pcb->filename);
  free(cur->pcb);
  free(cur);
}

void ready_queue_add_to_tail(struct QueueNode *node)
{
  // add the program to list of nodes if not already in list and there is space
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

// increase the age of all frames in every PCB by 1
void age_all_nodes()
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
          curr_node->pcb->page_table[j].last_used = curr_node->pcb->page_table[j].last_used + 1;
        }
      }
    }
  }
}

// print the ages of all the frames of each PCB node
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
          printf("program %s, frame %d, age %d\n", curr_node->pcb->progname, curr_node->pcb->page_table[j].loc, curr_node->pcb->page_table[j].last_used);
        }
      }
    }
  }
}

// iterate through list of all PCB nodes and their frames to find the least recently used
// honestly not the most efficient way of managing the lru but it does work at least
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
      for (int j = 0; j < curr_node->pcb->page_table_size; j++)
      {
        if (curr_node->pcb->page_table[j].last_used > max_age)
        {
          max_age = curr_node->pcb->page_table[j].last_used;
          lru->victimFrame = curr_node->pcb->page_table[j].loc;
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
  struct Page *page = node->pcb->page_table;

  for (int i = 0; i < node->pcb->page_table_size; i++)
  {
    if (page[i].loc != -1)
    {
      mem_free_lines_between(page[i].loc * 3, (page[i].loc * 3) + 2);
    }
  }

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