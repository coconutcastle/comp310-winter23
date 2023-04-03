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
PCB* makePCB(int start, int end){
    PCB * newPCB = malloc(sizeof(PCB));
    newPCB->pid = generatePID();
    newPCB->PC = start;
    newPCB->start  = start;
    newPCB->end = end;
    newPCB->job_length_score = 1+end-start;
    newPCB->priority = false;

    // create frame table, all invalid for now
    for (int i = 0; i < 10; i++) {
      newPCB -> page_table[i].valid = -1;
      newPCB -> page_table[i].frame = -1;
    }

    return newPCB;
}