#ifndef READY_QUEUE_H
#define READY_QUEUE_H
#include <pthread.h>
#include "pcb.h"

#define MAX_INT 2147483646

struct QueueNode {
    struct PCB *pcb;
    struct QueueNode *next;
};

struct LRU_frame {
  struct PCB *pcb;
  int page_index;
  int victimFrame;
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
void age_all_nodes();
struct LRU_frame *find_lru();
void print_node_ages();

#endif