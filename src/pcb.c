#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pcb.h"

int pid_counter = 1;

int generatePID()
{
  return pid_counter++;
}

// In this implementation, Pid is the same as file ID
struct PCB *makePCB(int num_lines, char *filename, char *progname, int page_table_size)
{
  struct PCB *newPCB = malloc(sizeof(struct PCB));
  newPCB->pid = generatePID();
  newPCB->program_counter = 0;
  newPCB->filename = strdup(filename);
  newPCB->progname = strdup(progname);
  newPCB->num_lines = num_lines;
  newPCB->priority = false;
  newPCB->page_table_size = 10;
  newPCB->num_blank_lines = 0;
  newPCB->last_frame = 0;

  // printf("made pcb with numlines %d\n", num_lines);

  // create frame table, all invalid for now
  for (int i = 0; i < newPCB->page_table_size; i++)
  {
    newPCB->page_table[i].valid = -1;
    newPCB->page_table[i].frame = -1;
    newPCB->page_table[i].last_used = -1;
  }

  return newPCB;
}

// return frame number of lru in pcb
int find_lru_frame(struct PCB *pcb) {
  for (int i = 0; i < 10; i++) {

  }
}