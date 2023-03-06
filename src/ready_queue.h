#ifndef READY_QUEUE
#define READY_QUEUE

#include "processControlBlock.h"

struct PCBreadyqueue {
  int queue_start;
  int queue_end;
  int size;
  int capacity;
  struct PCB* queue_array;
};

struct PCBreadyqueue* create_ready_queue(int capacity);
int enqueue(struct PCBreadyqueue* queue, struct PCB new_pcb);
struct PCB dequeue(struct PCBreadyqueue* queue);
struct PCB peek_front(struct PCBreadyqueue *queue);
struct PCB peek_end(struct PCBreadyqueue *queue);
int is_empty(struct PCBreadyqueue* queue);
int is_full(struct PCBreadyqueue* queue);
int run_ready_queue(struct PCBreadyqueue* queue, char *policy);
int run_PCB_FCFS(struct PCB pcb, struct PCBreadyqueue *queue);

#endif