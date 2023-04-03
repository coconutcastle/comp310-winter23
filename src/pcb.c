#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pcb.h"

int pid_counter = 1;

int generatePID(){
    return pid_counter++;
}

//In this implementation, Pid is the same as file ID 
struct PCB* makePCB(int num_lines){
    struct PCB * newPCB = malloc(sizeof(struct PCB));
    newPCB->pid = generatePID();
    newPCB->program_counter = 0;
    // newPCB->start  = start;
    // newPCB->end = end;
    newPCB->job_length_score = num_lines;
    newPCB->priority = false;

    // printf("made pcb with start stop %d %d\n", newPCB->PC, num_lines);

    // create frame table, all invalid for now
    for (int i = 0; i < 10; i++) {
      newPCB -> page_table[i].valid = -1;
      newPCB -> page_table[i].frame = -1;
    }

    return newPCB;
}