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

bool multi_threading = false;
pthread_t worker1;
pthread_t worker2;
bool active = false;
bool debug = false;
bool in_background = false;
pthread_mutex_t queue_lock;

void lock_queue()
{
  if (multi_threading)
    pthread_mutex_lock(&queue_lock);
}

void unlock_queue()
{
  if (multi_threading)
    pthread_mutex_unlock(&queue_lock);
}

int process_initialize(char *filename, char *prog_name, int num_lines)
{
  FILE *fp;
  int error_code = 0;

  fp = fopen(filename, "rt");
  if (fp == NULL)
  {
    error_code = 11; // 11 is the error code for file does not exist
    return error_code;
  }

  struct PCB *newPCB = makePCB(num_lines, filename, prog_name, 0);
  struct QueueNode *node = malloc(sizeof(struct QueueNode));
  node->pcb = newPCB;

  lock_queue();

  // store 2 pages into frame store, or 6 lines
  char *commands[6];

  int line_counter = get_lines_from_file(newPCB, commands, 6, 1);
  int blanks_counter = line_counter > 3 ? 6 - line_counter : 3 - line_counter;

  if (line_counter > 0)
  {
    int page_index_table = ((line_counter - 1) / 3); // will either be 0 or 1

    // now put full pages in memory
    for (int i = 0; i <= page_index_table; i++)
    {
      // get first free spot
      int page_index = get_free_page_frame();
      if (page_index != -1)
      {
        int insert_success = put_frame_in_memory(newPCB, commands, i, page_index, 1);
      }
    }
    newPCB->num_blank_lines = newPCB->num_blank_lines + blanks_counter;
  }

  ready_queue_add_to_tail(node);

  unlock_queue();
  fclose(fp);
  return error_code;
}

int shell_process_initialize()
{
  // Note that "You can assume that the # option will only be used in batch mode."
  // So we know that the input is a file, we can directly load the file into ram
  int *start = (int *)malloc(sizeof(int));
  int *end = (int *)malloc(sizeof(int));
  int error_code = 0;

  error_code = load_file(stdin, start, end, "_SHELL");
  if (error_code != 0)
  {
    return error_code;
  }

  struct PCB *newPCB = makePCB(end - start, "./", "", 0);
  newPCB->priority = true;
  struct QueueNode *node = malloc(sizeof(struct QueueNode));
  node->pcb = newPCB;
  lock_queue();

  ready_queue_add_to_head(node);

  unlock_queue();
  freopen("/dev/tty", "r", stdin);
  return 0;
}

bool execute_process(struct QueueNode *node, int quanta)
{
  char *line = NULL;
  struct PCB *pcb = node->pcb;
  int i = 0;

  while (i < quanta && (pcb->program_counter < pcb->num_lines + pcb->num_blank_lines))
  {
    int page_num = (pcb->program_counter) / 3;
    struct Page *curr_page = &(pcb->page_table[page_num]);

    if (curr_page->frame != -1)
    {
      // age all other frames in all PCBs, then set current frame age to 0
      age_all_nodes();
      curr_page->last_used = 0;

      int mem_loc = ((curr_page->frame) * 3) + ((pcb->program_counter) % 3);
      line = mem_get_value_at_line(mem_loc);

      if (strlen(line) > 0 && strcmp(line, "none") != 0) // check if blank line or invalid line return
      {
        in_background = true;
        if (pcb->priority)
        {
          pcb->priority = false;
        }

        if ((pcb->program_counter) >= (pcb->num_lines + pcb->num_blank_lines))
        {
          parseInput(line);
          in_background = false;
          return true;
        }

        parseInput(line);
        in_background = false;
      }
      pcb->program_counter = pcb->program_counter + 1;
      i++;
    }
    else
    {
      // if the pc is numlines + numblanks, then program is finished
      // otherwise, it's a page fault
      if (pcb->program_counter == (pcb->num_blank_lines + pcb->num_lines))
      {
        return true;
      }
      else
      {
        // get the missing page (one single page, 3 lines)
        char *commands[3];

        int line_counter = get_lines_from_file(pcb, commands, 3, 0);
        int blanks_counter = 3 - line_counter;

        // get first free spot
        int page_index = get_free_page_frame();
        if (page_index != -1)
        {
          int insert_success = put_frame_in_memory(pcb, commands, pcb->program_counter / 3, page_index, 0);
          pcb->num_blank_lines = pcb->num_blank_lines + blanks_counter;
        }
        else
        {
          // find lru frame
          struct LRU_frame *lru = find_lru();
          lru = find_lru();
          int victimFrame = lru->victimFrame;
          struct PCB *whichPCB = lru->pcb;
          int whichIndex = lru->page_index;

          printf("%s\n\n", "Page fault! Victim page contents:");

          for (int v = 0; v < 3; v++)
          {
            char *victim_line = mem_get_value_at_line((victimFrame * 3) + v);

            if (strlen(victim_line) > 0 && strcmp(victim_line, "none") != 0)
            {
              printf("%s", victim_line);
            }
          }
          printf("\n%s\n", "End of victim page contents.");

          whichPCB->page_table[whichIndex].frame = -1;
          whichPCB->page_table[whichIndex].last_used = -1;

          // evict frame
          mem_free_lines_between(victimFrame * 3, (victimFrame * 3) + 2);

          // load new frame into evicted frame spot
          for (int v = 0; v < 3; v++)
          {
            mem_set_by_index((victimFrame * 3) + v, pcb->progname, commands[v]);
          }

          pcb->page_table[pcb->program_counter / 3].frame = victimFrame;
          pcb->page_table[pcb->program_counter / 3].last_used = 0;

          free(lru);
        }
        return false;
      }
    }
  }
  if (pcb->program_counter >= pcb->num_lines + pcb->num_blank_lines)
  {
    return true;
  }
  return false;
}

void *scheduler_FCFS()
{
  struct QueueNode *cur;
  while (true)
  {
    lock_queue();
    if (is_ready_empty())
    {
      unlock_queue();
      if (active)
        continue;
      else
        break;
    }
    cur = ready_queue_pop_head();
    unlock_queue();
    execute_process(cur, MAX_INT);
  }
  if (multi_threading)
    pthread_exit(NULL);
  return 0;
}

void *scheduler_SJF()
{
  struct QueueNode *cur;
  while (true)
  {
    lock_queue();
    if (is_ready_empty())
    {
      unlock_queue();
      if (active)
        continue;
      else
        break;
    }
    cur = ready_queue_pop_shortest_job();
    unlock_queue();
    execute_process(cur, MAX_INT);
  }
  if (multi_threading)
    pthread_exit(NULL);
  return 0;
}

void *scheduler_AGING_alternative()
{
  struct QueueNode *cur;
  while (true)
  {
    lock_queue();
    if (is_ready_empty())
    {
      unlock_queue();
      if (active)
        continue;
      else
        break;
    }
    cur = ready_queue_pop_shortest_job();
    ready_queue_decrement_job_length_score();
    unlock_queue();
    if (!execute_process(cur, 1))
    {
      lock_queue();
      ready_queue_add_to_head(cur);
      unlock_queue();
    }
  }
  if (multi_threading)
    pthread_exit(NULL);
  return 0;
}

void *scheduler_AGING()
{
  struct QueueNode *cur;
  int shortest;
  sort_ready_queue();
  while (true)
  {
    lock_queue();
    if (is_ready_empty())
    {
      unlock_queue();
      if (active)
        continue;
      else
        break;
    }
    cur = ready_queue_pop_head();
    shortest = ready_queue_get_shortest_job_score();
    if (shortest < cur->pcb->job_length_score)
    {
      ready_queue_promote(shortest);
      ready_queue_add_to_tail(cur);
      cur = ready_queue_pop_head();
    }
    ready_queue_decrement_job_length_score();
    unlock_queue();
    if (!execute_process(cur, 1))
    {
      lock_queue();
      ready_queue_add_to_head(cur);
      unlock_queue();
    }
  }
  if (multi_threading)
    pthread_exit(NULL);
  return 0;
}

void *scheduler_RR(void *arg)
{
  int quanta = ((int *)arg)[0];
  struct QueueNode *cur;
  while (true)
  {
    lock_queue();
    if (is_ready_empty())
    {
      unlock_queue();
      if (active)
      {
        continue;
      }
      else
        break;
    }
    cur = ready_queue_pop_head();
    unlock_queue();
    if (!execute_process(cur, quanta))
    {
      lock_queue();
      ready_queue_add_to_tail(cur); // if program not finished, stick it back into the queue
      unlock_queue();
    }
  }
  if (multi_threading)
    pthread_exit(NULL);
  return 0;
}

int threads_initialize(char *policy)
{
  active = true;
  multi_threading = true;
  int arg[1];
  pthread_mutex_init(&queue_lock, NULL);
  if (strcmp("FCFS", policy) == 0)
  {
    pthread_create(&worker1, NULL, scheduler_FCFS, NULL);
    pthread_create(&worker2, NULL, scheduler_FCFS, NULL);
  }
  else if (strcmp("SJF", policy) == 0)
  {
    pthread_create(&worker1, NULL, scheduler_SJF, NULL);
    pthread_create(&worker2, NULL, scheduler_SJF, NULL);
  }
  else if (strcmp("RR", policy) == 0)
  {
    arg[0] = 2;
    pthread_create(&worker1, NULL, scheduler_RR, (void *)arg);
    pthread_create(&worker2, NULL, scheduler_RR, (void *)arg);
  }
  else if (strcmp("AGING", policy) == 0)
  {
    pthread_create(&worker1, NULL, scheduler_AGING, (void *)arg);
    pthread_create(&worker2, NULL, scheduler_AGING, (void *)arg);
  }
  else if (strcmp("RR30", policy) == 0)
  {
    arg[0] = 30;
    pthread_create(&worker1, NULL, scheduler_RR, (void *)arg);
    pthread_create(&worker2, NULL, scheduler_RR, (void *)arg);
  }
}

void threads_terminate()
{
  if (!active)
    return;
  bool empty = false;
  while (!empty)
  {
    empty = is_ready_empty();
  }
  active = false;
  pthread_join(worker1, NULL);
  pthread_join(worker2, NULL);
}

int schedule_by_policy(char *policy, bool mt)
{
  if (strcmp(policy, "FCFS") != 0 && strcmp(policy, "SJF") != 0 &&
      strcmp(policy, "RR") != 0 && strcmp(policy, "AGING") != 0 && strcmp(policy, "RR30") != 0)
  {
    return 15;
  }
  if (active)
    return 0;
  if (in_background)
    return 0;
  int arg[1];
  if (mt)
    return threads_initialize(policy);
  else
  {
    if (strcmp("FCFS", policy) == 0)
    {
      scheduler_FCFS();
    }
    else if (strcmp("SJF", policy) == 0)
    {
      scheduler_SJF();
    }
    else if (strcmp("RR", policy) == 0)
    {
      arg[0] = 2;
      scheduler_RR((void *)arg);
    }
    else if (strcmp("AGING", policy) == 0)
    {
      scheduler_AGING();
    }
    else if (strcmp("RR30", policy) == 0)
    {
      arg[0] = 30;
      scheduler_RR((void *)arg);
    }
    return 0;
  }
}

// returns the number of lines copied;
int get_lines_from_file(struct PCB *pcb, char *commands[], int max_lines, int condition)
{
  FILE *file = fopen(pcb->filename, "rt");

  // get the missing page (one single page)
  int all_lines_counter = 0;
  int line_counter = 0;
  char command[100];

  // go through all lines in file
  while (fgets(command, sizeof(command), file) != NULL)
  {
    all_lines_counter++;

    // where to start copying lines changes depending if part of initial frame add or after during execute
    // if condition = 1, adding frame for first time, only load the first 2 pages (2 * 3 lines = 6 lines)
    // if condition = 0, adding in frame after the fact and need to start adding lines after the already executed ones
    bool eval_cond = condition == 0 ? (all_lines_counter > pcb->program_counter && line_counter < 3) : (line_counter < max_lines && line_counter <= pcb->num_lines);
    if (eval_cond)
    {
      line_counter++;
      commands[line_counter - 1] = strdup(command);
    }
  }

  fclose(file);

  if (line_counter > 0 && line_counter % 3 != 0)
  {
    int page_num = ((line_counter - 1) / 3); // will either be 0 or 1
    for (int c = line_counter; c < (page_num + 1) * 3; c++)
    {
      commands[c] = "\0";
    }
  }
  return line_counter;
}

int put_frame_in_memory(struct PCB *pcb, char *commands[], int frame_index, int mem_loc, int is_new)
{
  char page_name[20];
  snprintf(page_name, 20, "-%d", frame_index);
  int commands_start = is_new == 0 ? 0 : frame_index; // if part of initial insert or while executing proess

  // put all lines for this page in memory
  for (int j = 0; j < 3; j++)
  {
    char page_line_name[100];
    snprintf(page_line_name, 100, "%s%s-%d", pcb->progname, page_name, j);

    mem_set_by_index(mem_loc + j, page_line_name, commands[(commands_start * 3) + j]);
  }

  pcb->page_table[frame_index].frame = mem_loc / 3;
  pcb->page_table[frame_index].last_used = 0;
}