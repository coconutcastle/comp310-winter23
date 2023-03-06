#ifndef PROCESS_CONTROL_BLOCK
#define PROCESS_CONTROL_BLOCK

struct PCB
{
  int pid;
  int script_location_start;
  int script_location_end;
  int num_instructions;
  int current_instruction;
};

struct PCB *create_PCB(int pid, FILE *commandLines);

#endif