#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "pcb.h"
#include "kernel.h"
#include "shell.h"
#include "shellmemory.h"
#include "interpreter.h"
#include "ready_queue.h"

struct QueueNode *head = NULL;

struct LRU_Node *LRU_head = NULL;
struct LRU_Node *LRU_tail = NULL;

int init_LRU() {
  struct QueueNode *tempNode = head;

  struct LRU_Node *tempLRU = NULL;
  struct LRU_Node *newLRUNode = NULL;

  LRU_head = NULL;
  LRU_tail = NULL;

  // add all progs in ready queue to LRU

  while (tempNode != NULL) {
    newLRUNode = (struct LRU_Node*)malloc(sizeof(struct LRU_Node));
    newLRUNode->pid = tempNode->pcb->pid;
    newLRUNode->last_used = -1;

    if (LRU_head == NULL) {
      LRU_head = newLRUNode;
      LRU_tail = newLRUNode;
    }
    else {
      newLRUNode->next_node = LRU_head;
      LRU_head->previous_node = newLRUNode;
      LRU_head = newLRUNode;
    }

    tempNode = tempNode->next;
  }
  return 0;
}

// put the most recently used at the front, so the LRU is at the back
// return head
void sort_LRU() {
  struct LRU_Node *curr = LRU_head;
  struct LRU_Node *tempNode = NULL;
  struct LRU_Node *insert = NULL;

  struct LRU_Node *newHead;
  struct LRU_Node *newTail;

  int youngest_age = 20;    // arbitrarily picked this number

  // the longer one goes unused, the higher the last_used count

  while (curr != NULL) {
    if (curr -> last_used < youngest_age) {
      youngest_age = curr -> last_used;
    }
    curr = curr -> next_node;
  }

  newHead = newTail = NULL;

  curr = LRU_head;
  while (curr != NULL) {
    tempNode = (struct LRU_Node *)malloc(sizeof(struct LRU_Node));
    tempNode->page_num = curr -> page_num;
    tempNode->page_table_index = curr -> page_table_index;
    tempNode->next_node = NULL;
    tempNode->previous_node = NULL;

    insert = newHead;
    while (insert != NULL && insert -> last_used < tempNode->last_used) {
      insert = insert -> next_node;
    }

    if (insert != NULL) {
      tempNode -> previous_node = insert -> previous_node;
      tempNode -> next_node = insert;
      
      if (insert -> previous_node != NULL) {
        insert ->previous_node->next_node = tempNode;
      }
      else {
        newHead = tempNode;
      }

      insert->previous_node = tempNode;
    }
    else {
      if (newHead == NULL) {
        newHead = newTail = tempNode;
      }
      else {
        tempNode -> next_node = newHead;
        newHead -> previous_node = tempNode;
        newHead = tempNode;
      }
    }
    curr = curr -> next_node;
  }

  LRU_head = newHead;
  LRU_tail = newTail;
  while (LRU_tail != NULL && LRU_tail->next_node != NULL) {
    LRU_tail = LRU_tail->next_node;
  }
}

struct LRU_Node *get_LRU() {
  return LRU_tail;
}

void ready_queue_destory()
{
    if(!head) return;
    struct QueueNode *cur = head;
    struct QueueNode *tmp;
    while(cur->next!=NULL){
        tmp = cur->next;
        free(cur);
        cur = tmp;
    }
    free(cur);
}

void ready_queue_add_to_tail(struct QueueNode *node)
{
    if(!head){
        head = node;
        head->next = NULL;
    }
    else{
        struct QueueNode *cur = head;
        while(cur->next!=NULL) cur = cur->next;
        cur->next = node;
        cur->next->next = NULL;
    }
}

void ready_queue_add_to_head(struct QueueNode *node)
{
    if(!head){
        head = node;
        head->next = NULL;
    }
    else{
        node->next = head;
        head = node;
    }
}

void print_ready_queue(){
    if(!head) {
        printf("%s\n", "ready queue is empty");
        return;
    }
    struct QueueNode *cur = head;
    printf("%s\n","Ready queue: ");
    while(cur!=NULL){
        printf("\tPID: %d, length: %d, priority: %d, pc: %d\n", cur->pcb->pid, cur->pcb->num_lines, cur->pcb->priority, cur->pcb->program_counter);
        cur = cur->next;
    }
}

void terminate_process(struct QueueNode *node){
    //node should not be in the ready queue

    // get all pages associated with process, always in chunks of 3
    struct PTE *pte = node -> pcb -> page_table;

    for (int i = 0; i < 10; i++) {
      if (pte[i].valid == 1) {
        mem_free_lines_between(pte[i].frame * 3, (pte[i].frame * 3) + 2);
      }
    }

    // printf("%s\n", "terminated");

    // printShellMemory();

    // mem_free_lines_between(node->pcb->start, node->pcb->end);

    free(node);
}

bool is_ready_empty(){
    return head==NULL;
}

struct QueueNode *ready_queue_pop_head(){
    struct QueueNode *tmp = head;
    if(head!=NULL) head = head->next;
    // printf("popped %d at %d\n", tmp->pcb->pid, tmp->pcb->program_counter);
    return tmp;
}

void ready_queue_decrement_job_length_score(){
    struct QueueNode *cur;
    cur = head;
    while(cur!=NULL){
        if(cur->pcb->job_length_score>0) cur->pcb->job_length_score--;
        cur=cur->next;
    }
}

void ready_queue_swap_with_next(struct QueueNode *toSwap){
    struct QueueNode *next;
    struct QueueNode *afterNext;
    struct QueueNode *cur = head;
    if(head==toSwap){
        next = head->next;
        head->next = next->next;
        next->next = head;
        head = next;
    }
    while(cur!=NULL && cur->next!=toSwap) cur = cur->next;
    if(cur==NULL) return;
    next = cur->next->next;
    afterNext = next->next;
    //cur toSwap next afterNext
    cur->next = next;
    next->next = toSwap;
    toSwap->next = afterNext;
}

bool swap_needed(struct QueueNode *cur){
    struct QueueNode *next = cur->next;
    if(!next) return false;
    if(cur->pcb->priority && next->pcb->priority){
        if(cur->pcb->job_length_score > next->pcb->job_length_score){
            return true;
        }
        else return false;
    }
    else if(cur->pcb->priority && !next->pcb->priority) return false;
    else if(!cur->pcb->priority && next->pcb->priority) return true;
    else{
        if(cur->pcb->job_length_score > next->pcb->job_length_score) return true;
        else return false;
    }
}

void sort_ready_queue(){
    if(head==NULL) return;
    //bubble sort
    struct QueueNode *cur = head;
    bool sorted = false;
    while(!sorted){
        sorted = true;
        while(cur->next!=NULL){
            if(swap_needed(cur)){
                sorted = false;
                ready_queue_swap_with_next(cur);
            }
            else{
                cur=cur->next;
            }
        }
    }
}

struct QueueNode *ready_queue_pop_shortest_job(){
    sort_ready_queue();
    struct QueueNode *node = ready_queue_pop_head();
    return node;
}

int ready_queue_get_shortest_job_score(){
    struct QueueNode *cur  = head;
    int shortest = MAX_INT;
    while(cur!=NULL){
        if(cur->pcb->job_length_score<shortest) {
            shortest = cur->pcb->job_length_score;
        }
        cur = cur->next;
    }
    return shortest;
}

void ready_queue_promote(int score){
    if(head->pcb->job_length_score == score) return;
    struct QueueNode *cur = head;
    struct QueueNode *next;
    while(cur->next!=NULL){
        if(cur->next->pcb->job_length_score == score) break;
        cur = cur->next;
    }
    if(cur->next==NULL || cur->next->pcb->job_length_score!=score) return;
    next = cur->next;
    cur->next = cur->next->next;
    next->next = head;
    head = next;
}