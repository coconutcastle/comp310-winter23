#ifndef PCB_H
#define PCB_H
#include <stdbool.h>

struct PTE
{
  int frame;
  int valid;
  int last_used;    // the longer its been since its been accessed, the larger the number gets
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
  int num_blank_lines;
  int mem_start;
  // int start;
  // int end;
  int job_length_score;
  char *filename;
  char *progname;
  int page_table_size;

  // page table
  struct PTE page_table[10];
};

// struct Page
// {
//   char *lines[3];
// }

int generatePID();
struct PCB *makePCB(int numlines, char *filename, char *progname, int page_table_size);
#endif