#ifndef PCB_H
#define PCB_H
#include <stdbool.h>

struct PTE
{
  int frame;
  int valid;
  int age;
  char *script_id;
};

/*
 * Struct:  PCB
 * --------------------
 * pid: process(task) id
 * PC: program counter, stores the index of line that the task is executing
 * start: the first line in shell memory that belongs to this task
 * end: the last line in shell memory that belongs to this task
 * job_length_score: for EXEC AGING use only, stores the job length score
 */
struct PCB
{
  bool priority;
  int pid;
  int program_counter;
  int num_lines;
  int mem_start;
  // int start;
  // int end;
  int job_length_score;

  // page table
  struct PTE page_table[10];
};

// struct Page
// {
//   char *lines[3];
// }

int generatePID();
struct PCB *makePCB(int numlines);
#endif