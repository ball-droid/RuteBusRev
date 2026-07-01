/*
 * QUEUE ADT — Header
 * ===================
 * Queue generic (void*) dengan implementasi LINKED LIST.
 * Fully dynamic — tanpa batas kapasitas.
 *
 * Digunakan oleh nbtree.c (bfsTree) untuk antrian BFS.
 * Tidak tergantung pada modul lain — independen.
 *
 * Operasi:
 *   enqueue()  : malloc QNode baru → tambah ke rear
 *   dequeue()  : ambil dari front → free QNode
 *   freeQueue(): dequeue semua (free semua QNode)
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct QNode {
    void* data;
    struct QNode* next;
} QNode;

typedef struct {
    QNode* front;
    QNode* rear;
    int count;
} Queue;

void  initQueue(Queue* q);
void  enqueue(Queue* q, void* item);
void* dequeue(Queue* q);
bool  isQueueEmpty(Queue* q);
int   queueSize(Queue* q);
void  freeQueue(Queue* q);

#endif
