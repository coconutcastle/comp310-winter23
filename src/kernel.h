#ifndef KERNEL
#define KERNEL
#include "pcb.h"

int process_initialize(char *filename, char *prog_name, int num_lines);
int schedule_by_policy(char* policy, bool mt);
int shell_process_initialize();
void ready_queue_destory();
void threads_terminate();
int put_frame_in_memory(struct PCB *pcb, char *lines[], int frame_index, int mem_loc, int is_new);
int get_lines_from_file(struct PCB *pcb, char *commands[], int max_lines, int condition);
#endif