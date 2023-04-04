#ifndef READY_QUEUE_H
#define READY_QUEUE_H
#include <pthread.h>
#include "pcb.h"

#define MAX_INT 2147483646

struct QueueNode {
    struct PCB *pcb;
    struct QueueNode *next;
};

struct LRU_Node {
  int frame_num;
  char *prog_name;
  int pid;
  int age;
  struct LRU_Node *next;    // most recently accessed will be at the front
};

void ready_queue_destory();
void ready_queue_add_to_tail(struct QueueNode *node);
void print_ready_queue();
void terminate_process(struct QueueNode *node);
bool is_ready_empty();
struct QueueNode *ready_queue_pop_shortest_job();
void ready_queue_head_to_tail();
void ready_queue_add_to_head(struct QueueNode *node);
struct QueueNode *ready_queue_pop_head();
void ready_queue_decrement_job_length_score();
void sort_ready_queue();
int ready_queue_get_shortest_job_score();
void ready_queue_promote(int score);

#endif