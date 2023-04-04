#ifndef KERNEL
#define KERNEL
#include "pcb.h"

int process_initialize(char *filename, char *prog_name, int num_lines);
int schedule_by_policy(char* policy, bool mt);
int shell_process_initialize();
void ready_queue_destory();
void threads_terminate();
#endif