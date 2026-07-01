/*
 * NBTREE ADT — N-ary Tree (Non-Binary Tree)
 * ==========================================
 * Modul ini mengelola struktur data N-ary Tree dan algoritma BFS.
 *
 * TRAVERSAL ALGORITHMS:
 *   ┌─────────────────────┬──────────────────────┬──────────────────────────────┐
 *   │ Fungsi              │ Traversal            │ Pola                         │
 *   ├─────────────────────┼──────────────────────┼──────────────────────────────┤
 *   │ buildNaryTree()     │ DFS Pre-order        │ parent → firstSon → nextBro  │
 *   │                     │ (Depth-First Search) │ (bor child dulu, lalu sibling)│
 *   ├─────────────────────┼──────────────────────┼──────────────────────────────┤
 *   │ bfsTree()           │ BFS Level-order      │ level per level via Queue    │
 *   │                     │ (Breadth-First Search│ parent + firstSon chain      │
 *   ├─────────────────────┼──────────────────────┼──────────────────────────────┤
 *   │ printNodeRecursive()│ DFS Pre-order        │ cetak parent dulu,           │
 *   │                     │                      │ lalu firstSon → nextBrother  │
 *   └─────────────────────┴──────────────────────┴──────────────────────────────┘
 *
 * DETAIL PRE-ORDER PADA N-ARY TREE:
 *   1. Proses node saat ini (parent)
 *   2. Rekursi ke firstSon (child pertama)
 *   3. Setelah seluruh subtree firstSon selesai, rekursi ke nextBrother
 *   4. nextBrother menunggu sampai semua child firstSon habis (DFS)
 *
 * STRUKTUR NODE:
 *   - firstSon     : anak pertama          (↓ turun)
 *   - nextBrother  : saudara berikutnya    (→ linked list antar sibling)
 *   - parent       : orang tua             (↑ naik, untuk BFS)
 *
 * DFS SPANNING TREE (buildNaryTree):
 *   1. DFS dari root (Pool Damri)
 *   2. Tetangga ditemukan dengan SCAN KORIDOR — tanpa graph adjacency
 *   3. Setiap tetangga yang belum dikunjungi menjadi CHILD
 *   4. addChild() mengisi firstSon/nextBrother/parent
 *   5. Hasil: spanning tree — karena graf bus adalah tree (no cycle)
 *
 * BFS (bfsTree):
 *   - Traversal via PARENT (atas) + firstSon chain (bawah)
 *   - Queue ADT (queue.h/c) untuk antrian — enqueue/dequeue linked list
 *   - Menjamin rute terpendek (BFS standard pada unweighted graph)
 *
 * DEPENDENSI:
 *   - nbtree.c tergantung pada: queue.h (untuk BFS internal)
 *   - nbtree.h digunakan oleh: data.h, main.c
 *   - queue.h independen (generic void*)
 */
#ifndef NBTREE_H
#define NBTREE_H

#include <stdio.h>
#include <stdbool.h>

/* MAX_NAME masih diperlukan untuk buffer baca file dan display */
#define MAX_NAME 100

/* ============================================================
 * NODE - N-ARY TREE (fully dynamic, malloc per node)
 * ============================================================ */
typedef struct Node {
    char name[MAX_NAME];
    int  idx;
    struct Node* firstSon;
    struct Node* nextBrother;
    struct Node* parent;
} Node;

/* ============================================================
 * KORIDOR - dynamic array of strings
 * names: char** (setiap string = malloc)
 * size : jumlah halte saat ini
 * capacity : kapasitas names array
 * ============================================================ */
typedef struct {
    char** names;
    int size;
    int capacity;
} Koridor;

/* ============================================================
 * FUNGSI-FUNGSI N-ARY TREE
 * ============================================================ */
Node* createNode(const char* name, int idx);
void  addChild(Node* parent, Node* child);
void  freeNode(Node* node);

int   findNodeIndex(Node** nodes, int nodeCount, const char* name);

void  buildNaryTree(Node* curr, bool visited[],
                    Koridor* koridors, int koridorCount,
                    Node** nodes, int nodeCount);

void  printNodeRecursive(FILE* file, Node* node,
                         const char* prefix, bool isLast,
                         char** nodeInitials);

int   bfsTree(Node* start, Node* dest,
              Node** path, int* pathLen,
              Node** nodes, int nodeCount);

#endif
