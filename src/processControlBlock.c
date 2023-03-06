#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "shellmemory.h"
#include "shell.h"
#include "processControlBlock.h"

struct PCB *create_PCB(int pid, FILE *commandLines)
{
  // find memory block that will fit the commands
  // assuming each script has <= 100 lines of source code
  int pcb_start_loc = mem_find_space(100);
  if (pcb_start_loc == -1)
  {
    printf("No space.");
  }

  struct PCB *pcb = malloc(sizeof(struct PCB));

  char line[100];
  int line_index = 0;
  int num_commands = 0;

  if (fgets(line, sizeof(line), commandLines) != NULL)
  {
    // split commands by newline, add to memory one line at a time
    char *command = strtok(line, (char *)"\n");
    while (command != NULL)
    {
      // memory key will be <pid>-<#instruction>
      // find specific instruction by pcb start + instruction index
      char mem_key[100];
      sprintf(mem_key, "%d-%d", pcb->pid, num_commands);

      mem_set_command_value(pcb_start_loc + num_commands, command, mem_key);

      num_commands++;

      // printf("added %d", num_commands);

      command = strtok(NULL, (char *)"\n");
    }

    memset(line, 0, sizeof(line));
  }

  pcb->pid = 1;
  pcb->script_location_start = pcb_start_loc;
  pcb->script_location_end = pcb_start_loc + 100;
  pcb->num_instructions = num_commands;
  pcb->current_instruction = 0;

  return pcb;
}