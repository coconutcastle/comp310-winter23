#ifndef KERNEL
#define KERNEL
#include "pcb.h"

int process_initialize(char *filename, char *prog_name, int num_lines);
int schedule_by_policy(char* policy, bool mt);
int shell_process_initialize();
void ready_queue_destory();
void threads_terminate();
int insert_frame(FILE *fp, struct PCB *pcb, char *prog_name, int max_lines, int num_lines, int condition);
int put_frame_in_memory(struct PCB *pcb, char *lines[], int frame_index, int mem_loc, int is_new);
#endif