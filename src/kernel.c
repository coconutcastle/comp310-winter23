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

// int process_initialize(char *filename, char *prog_name, int num_lines)
// {
//   FILE *fp;
//   int error_code = 0;
//   // int *start = (int *)malloc(sizeof(int));
//   // int *end = (int *)malloc(sizeof(int));

//   printf("%s\n","in p initialize");

//   fp = fopen(filename, "rt");
//   if (fp == NULL)
//   {
//     error_code = 11; // 11 is the error code for file does not exist
//     return error_code;
//   }

//   // error_code = load_file(fp, start, end, filename);
//   // if (error_code != 0)
//   // {
//   //   fclose(fp);
//   //   return error_code;
//   // }

//   struct PCB *newPCB = makePCB(num_lines);
//   struct QueueNode *node = malloc(sizeof(struct QueueNode));
//   node->pcb = newPCB;

//   lock_queue();

//   int total_lines_stored = 0;

//   // store 2 pages into frame store
//   for (int page_num = 0; page_num < 2; page_num++)
//   {
//     char *lines[3];

//     int line_counter = 0;
//     char line[101];
//     int page_line_counter = 0; // number of lines stored

//     // go through all lines in file
//     while (fgets(line, sizeof(line), fp) != NULL)
//     {

//       // if number of lines so far within bounds, add to page
//       if ((line_counter >= page_num * 3) && (line_counter < (page_num + 1) * 3))
//       {
//         lines[line_counter % 3] = strdup(line);
//         page_line_counter++;
//       }
//       line_counter++;
//       memset(line, 0, sizeof(line));
//     }

//     if (page_line_counter == 0)
//     {
//       break;
//     }

//     // if number of lines stored hasn't reached 3, pad with empty lines
//     if (page_line_counter < 3)
//     {
//       for (int i = page_line_counter; i < 3; i++)
//       {
//         lines[i] = "\0";
//       }
//     }

//     // now put full page in memory

//     // get first free spot
//     int page_index = get_free_page_frame();

//     if (page_index != -1)
//     {
//       char page_name[3];
//       snprintf(page_name, 20, "-%d-", page_num);

//       for (int i = 0; i < 3; i++)
//       {
//         char line_name[3];
//         char page_line_name[20];

//         snprintf(page_name, 20, "-%d-", i);

//         strcpy(page_line_name, prog_name);
//         strcat(page_line_name, page_name);
//         strcat(page_line_name, line_name);

//         mem_set_by_index(page_index + i, page_line_name, lines[i]);
//       }

//       newPCB->page_table[page_num].frame = page_index / 3;
//       newPCB->page_table[page_num].valid = 1;
//       newPCB->page_table[page_num].age = 0;

//       printf("done setting %d at %d\n", newPCB -> page_table[page_num].frame, page_num);
//     }
//     else
//     {
//       // no free space to put page, page fault
//     }
//   }

//   ready_queue_add_to_tail(node);

//   unlock_queue();
//   fclose(fp);
//   return error_code;
// }

int process_initialize(char *filename, char *prog_name, int num_lines)
{
  FILE *fp;
  int error_code = 0;
  // int *start = (int *)malloc(sizeof(int));
  // int *end = (int *)malloc(sizeof(int));

  // printf("%s\n", "in p initialize");

  fp = fopen(filename, "rt");
  if (fp == NULL)
  {
    error_code = 11; // 11 is the error code for file does not exist
    return error_code;
  }

  // error_code = load_file(fp, start, end, filename);
  // if (error_code != 0)
  // {
  //   fclose(fp);
  //   return error_code;
  // }

  struct PCB *newPCB = makePCB(num_lines);
  struct QueueNode *node = malloc(sizeof(struct QueueNode));
  node->pcb = newPCB;

  lock_queue();
  // store 2 pages into frame store

  char *lines[6];
  int line_counter = 0;
  int blanks_counter = 0;
  char line[101];

  // go through all lines in file
  while (fgets(line, sizeof(line), fp) != NULL)
  {
    if (line_counter < 6 && line_counter <= num_lines)
    {
      // only load the first 2 pages (2 * 3 lines = 6 lines)
      line_counter++;
      lines[line_counter - 1] = strdup(line);
      memset(line, 0, sizeof(line));
    }
  }

  if (line_counter > 0)
  {
    int page_index_table = ((line_counter - 1) / 3); // will either be 0 or 1

    // if number of lines stored hasn't reached 3, pad with empty lines
    if (line_counter % 3 != 0)
    {
      for (int i = line_counter; i < (page_index_table + 1) * 3; i++)
      {
        blanks_counter++;
        lines[i] = "\0";
      }
    }

    // now put full pages in memory

    for (int i = 0; i <= page_index_table; i++)
    {
      // get first free spot
      int page_index = get_free_page_frame();
      if (page_index != -1)
      {
        char page_name[20];
        snprintf(page_name, 20, "-%d", i);

        // put all lines for this page in
        for (int j = 0; j < 3; j++)
        {
          char line_name[20];
          char page_line_name[100];

          snprintf(line_name, 20, "-%d", j);

          strcpy(page_line_name, prog_name);
          strcat(page_line_name, page_name);
          strcat(page_line_name, line_name);

          // printf("setting to %s\n", page_line_name);
          // printf("set to %d %d\n", page_index, j);

          mem_set_by_index(page_index + j, page_line_name, lines[(i * 3) + j]);
        }

        newPCB->page_table[i].frame = page_index / 3;
        newPCB->page_table[i].valid = 1;

        // printf("pc is %d\n", newPCB->program_counter);

        // printf("done setting %d at %d w %d\n", newPCB->page_table[i].frame, i, page_index);
      }
      else
      {
        // no free space to put page, page fault
        
        // find least recently used page
        // init_LRU();
        // sort_LRU();

        // struct LRU_Node *victim = get_LRU();

        // printf("%s\n", "Page fault! Victim page contents:");

        // printf("%s\n", "End of victim page contents.");

        // struct LRU_PCB *frame_to_evict =
      }
    }
    newPCB->num_blank_lines = blanks_counter;
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

  struct PCB *newPCB = makePCB(end - start);
  newPCB->priority = true;
  struct QueueNode *node = malloc(sizeof(struct QueueNode));
  node->pcb = newPCB;
  lock_queue();

  ready_queue_add_to_head(node);

  unlock_queue();
  freopen("/dev/tty", "r", stdin);
  return 0;
}

// bool execute_process(struct QueueNode *node, int quanta)
// {
//   // printf("%s\n", "executing policy");
//   char *line = NULL;
//   struct PCB *pcb = node->pcb;
//   int frame;

//   int page_num = (pcb->program_counter) / 3;
//   struct PTE *curr_pte = &(pcb->page_table[page_num]);
//   // printf("got pte, and is %d %d\n", page_num, pcb->program_counter);

//   if (curr_pte->valid == 1)
//   {
//     // printf("mem loc is %d\n", mem_loc);
//     // printShellMemory();

//     // printf("valid %d\n", pte->frame);

//     for (int i = 0; i < quanta; i++)
//     {
//       int mem_loc = ((curr_pte->frame) * 3) + ((pcb->program_counter) % 3);

//       // printf("mem loc %d %d\n", mem_loc, pte->frame);
//       // line = mem_get_value_at_line(pcb->PC++);

//       line = mem_get_value_at_line(mem_loc);

//       // printf("Got line %s\n", line);

//       if (strlen(line) > 0)
//       {
//         in_background = true;

//         if (pcb->priority)
//         {
//           pcb->priority = false;
//         }

//         if ((pcb->program_counter) >= (pcb->num_lines))
//         {
//           parseInput(line);
//           terminate_process(node);
//           in_background = false;
//           return true;
//         }

//         // printf("%s\n", "hi");

//         parseInput(line);
//         in_background = false;
//         pcb->program_counter = pcb->program_counter + 1;
//       }
//     }
//   }

//   return false;
// }

bool execute_process(struct QueueNode *node, int quanta)
{
  // printf("%s\n", "executing policy");
  char *line = NULL;
  struct PCB *pcb = node->pcb;
  int frame;
  int i = 0;

  while (i < quanta)
  {
    int page_num = (pcb->program_counter) / 3;
    struct PTE *curr_pte = &(pcb->page_table[page_num]);
    // printf("got pte, and is %d %d\n", page_num, pcb->program_counter);

    if (curr_pte->valid == 1)
    {
      // printf("mem loc is %d\n", mem_loc);
      // printShellMemory();

      // printf("valid %d\n", pte->frame);

      // frame = 0;

      
        int mem_loc = ((curr_pte->frame) * 3) + ((pcb->program_counter) % 3);
        // printf("mem loc %d %d %d\n", mem_loc, curr_pte->frame, pcb->program_counter);
        // line = mem_get_value_at_line(pcb->PC++);

        line = mem_get_value_at_line(mem_loc);
        // printf("Got line %s\n", line);
        if (strlen(line) > 0 && strcmp(line, "none") != 0)
        {
          in_background = true;
          if (pcb->priority)
          {
            pcb->priority = false;
          }

          if ((pcb->program_counter) >= (pcb->num_lines + pcb ->num_blank_lines))
          {
            parseInput(line);
            terminate_process(node);
            in_background = false;
            return true;
          }

          // printf("%s\n", "hi");

          parseInput(line);
          in_background = false;
        }
        pcb->program_counter = pcb->program_counter + 1;
        // frame++;
        i++;
      
    }
    else {
      // printShellMemory();
      // print_ready_queue();
      terminate_process(node);
      return true;    // ugh this was the only thing that would make the program quit properly
    }
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
  // printf("%s\n", "in RR");
  // print_ready_queue();
  int quanta = ((int *)arg)[0];
  struct QueueNode *cur;
  // printf("%s\n", "entering");
  while (true)
  {
    // print_ready_queue();
    // printf("%d\n", 1);
    lock_queue();
    if (is_ready_empty())
    {
      // printf("%s\n", "already empty");
      unlock_queue();
      if (active)
      {
        // printf("%d\n", 2);
        continue;
      }
      else
        break;
    }
    // printf("%d\n", 3);
    cur = ready_queue_pop_head();
    // printf("cur is %d at %d\n", cur->pcb->page_table, cur->pcb->program_counter);
    unlock_queue();
    // printf("%d\n", 4);
    if (!execute_process(cur, quanta))
    {
      lock_queue();
      // printf("%s\n", "here");
      // printf("%d\n", 5);
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
  // printf("%s\n", "in scheduler");
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
      // print_ready_queue();
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
