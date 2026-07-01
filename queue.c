/*
 * QUEUE ADT — Implementation
 * ===========================
 * Implementasi Queue dengan singly linked list.
 *
 * - enqueue() : node baru selalu di REAR. O(1).
 * - dequeue() : node diambil dari FRONT. O(1).
 * - freeQueue(): dequeue hingga kosong (free semua QNode).
 *
 * Tidak ada batas kapasitas — selama malloc masih berhasil.
 */
#include <stdlib.h>
#include "queue.h"

void initQueue(Queue* q) {
    q->front = NULL;
    q->rear  = NULL;
    q->count = 0;
}

void enqueue(Queue* q, void* item) {
    QNode* node = (QNode*)malloc(sizeof(QNode));
    if (!node) return;
    node->data = item;
    node->next = NULL;

    if (q->rear == NULL) {
        q->front = node;
        q->rear  = node;
    } else {
        q->rear->next = node;
        q->rear = node;
    }
    q->count++;
}

void* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;

    QNode* temp = q->front;
    void* item = temp->data;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    q->count--;
    return item;
}

bool isQueueEmpty(Queue* q) {
    return q->count == 0;
}

int queueSize(Queue* q) {
    return q->count;
}

void freeQueue(Queue* q) {
    while (!isQueueEmpty(q))
        dequeue(q);
}
