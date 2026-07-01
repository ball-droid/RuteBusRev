/*
 * NBTREE ADT — Implementation
 * ============================
 * Implementasi fungsi-fungsi N-ary Tree dengan 3 traversal berbeda:
 *
 * ┌─────────────────────┬──────────────────────┬──────────────────────────────┐
 * │ Fungsi              │ Traversal            │ Cara kerja                   │
 * ├─────────────────────┼──────────────────────┼──────────────────────────────┤
 * │ buildNaryTree()     │ DFS PRE-ORDER        │ parent → firstSon            │
 * │                     │ (Depth-First Search) │ → child habis dulu           │
 * │                     │                      │ → nextBrother setelahnya     │
 * ├─────────────────────┼──────────────────────┼──────────────────────────────┤
 * │ bfsTree()           │ BFS LEVEL-ORDER      │ Queue, level per level       │
 * │                     │ (Breadth-First Search│ parent + firstSon chain      │
 * ├─────────────────────┼──────────────────────┼──────────────────────────────┤
 * │ printNodeRecursive()│ DFS PRE-ORDER        │ cetak parent, rekursi child  │
 * │                     │                      │ firstSon → nextBrother       │
 * └─────────────────────┴──────────────────────┴──────────────────────────────┘
 *
 * DETAIL TRAVERSAL:
 *   buildNaryTree() — DFS Pre-order:
 *     visited[curr] = true → cari tetangga via scan koridor
 *     → addChild(firstSon) → rekursi buildNaryTree(firstSon)
 *     → setelah semua anak firstSon selesai → addChild(nextBrother)
 *     → rekursi buildNaryTree(nextBrother) → ... (ulang)
 *
 *   bfsTree() — BFS Level-order:
 *     enqueue(start) → while(queue tidak kosong)
 *     → dequeue curr → PROCESS(curr->parent) → PROCESS(curr->firstSon chain)
 *     → PROCESS: jika belum visited → enqueue
 *     → jika node == dest → rekonstruksi path dari parentIdx[]
 *
 *   printNodeRecursive() — DFS Pre-order:
 *     cetak node → rekursi ke firstSon → setelah selesai, lanjut nextBrother
 *
 * createNode()       : alokasi 1 Node (heap)
 * addChild()         : tambah child via firstSon/nextBrother chain
 * findNodeIndex()    : cari index node by name (case-insensitive)
 */
#include "nbtree.h"
#include "queue.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ============================================================
 * createNode - Alokasi 1 node (heap)
 * ============================================================ */
Node* createNode(const char* name, int idx) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return NULL;
    strncpy(node->name, name, MAX_NAME - 1);
    node->name[MAX_NAME - 1] = '\0';
    node->idx         = idx;
    node->firstSon    = NULL;
    node->nextBrother = NULL;
    node->parent      = NULL;
    return node;
}

/* ============================================================
 * freeNode - Bebaskan 1 node (tidak include child)
 * ============================================================ */
void freeNode(Node* node) {
    free(node);
}

/* ============================================================
 * addChild - Tambah child via firstSon/nextBrother chain
 * ============================================================ */
void addChild(Node* parent, Node* child) {
    if (!parent || !child) return;
    child->parent = parent;
    if (parent->firstSon == NULL) {
        parent->firstSon = child;
    } else {
        Node* sib = parent->firstSon;
        while (sib->nextBrother != NULL)
            sib = sib->nextBrother;
        sib->nextBrother = child;
    }
}

/* ============================================================
 * ciEqual - Case-Insensitive string comparison (return true/false)
 * ============================================================ */
static bool ciEqual(const char* a, const char* b) {
    for (; *a && *b; a++, b++)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return false;
    return *a == *b;
}

/* ============================================================
 * findNodeIndex - Cari index node by name (CASE-INSENSITIVE)
 * ============================================================ */
int findNodeIndex(Node** nodes, int nodeCount, const char* name) {
    for (int i = 0; i < nodeCount; i++)
        if (ciEqual(nodes[i]->name, name)) return i;
    return -1;
}

/* ============================================================
 * buildNaryTree - DFS spanning tree, neighbor via scan koridor
 * Dynamic: neighbor list pakai realloc
 * ============================================================ */
void buildNaryTree(Node* curr, bool visited[],
                   Koridor* koridors, int koridorCount,
                   Node** nodes, int nodeCount) {
    if (!curr) return;
    visited[curr->idx] = true;

    /* Kumpulkan tetangga (dynamic array, tumbuh sesuai kebutuhan) */
    Node** neighbors = NULL;
    int nCount = 0, nCap = 0;

    for (int k = 0; k < koridorCount; k++) {
        for (int i = 0; i < koridors[k].size; i++) {
            if (strcmp(koridors[k].names[i], curr->name) == 0) {
                for (int d = -1; d <= 1; d += 2) {
                    int ni = i + d;
                    if (ni < 0 || ni >= koridors[k].size) continue;
                    int nIdx = findNodeIndex(nodes, nodeCount,
                                             koridors[k].names[ni]);
                    if (nIdx < 0 || visited[nIdx]) continue;

                    if (nCount >= nCap) {
                        nCap = nCap ? nCap * 2 : 4;
                        Node** tmp = (Node**)realloc(neighbors,
                                                     nCap * sizeof(Node*));
                        if (!tmp) { free(neighbors); return; }
                        neighbors = tmp;
                    }
                    neighbors[nCount++] = nodes[nIdx];
                }
            }
        }
    }

    /* Bangun child chain + rekursi */
    for (int i = 0; i < nCount; i++) {
        addChild(curr, neighbors[i]);
        buildNaryTree(neighbors[i], visited,
                      koridors, koridorCount, nodes, nodeCount);
    }
    free(neighbors);
}

/* ============================================================
 * printNodeRecursive - Cetak tree ke file (rekursif)
 * nodeInitials adalah array dynamic char**
 * ============================================================ */
void printNodeRecursive(FILE* file, Node* node,
                        const char* prefix, bool isLast,
                        char** nodeInitials) {
    if (!file || !node) return;

    fprintf(file, "%s%s%s\n", prefix,
            isLast ? "\342\224\224\342\224\200\342\224\200 "
                   : "\342\224\234\342\224\200\342\224\200 ",
            nodeInitials[node->idx]);

    char childPrefix[MAX_NAME * 2];
    snprintf(childPrefix, sizeof(childPrefix), "%s%s", prefix,
             isLast ? "    " : "\342\224\202   ");

    /* Hitung jumlah child */
    int total = 0;
    Node* tmp = node->firstSon;
    while (tmp) { total++; tmp = tmp->nextBrother; }

    /* Cetak setiap child */
    int cnt = 0;
    Node* ch = node->firstSon;
    while (ch) {
        cnt++;
        printNodeRecursive(file, ch, childPrefix, cnt == total, nodeInitials);
        ch = ch->nextBrother;
    }
}

/* ============================================================
 * bfsTree - BFS via tree pointers (parent + firstSon chain)
 * Queue menggunakan Queue ADT (linked list, dinamis)
 * visited[] dan parentIdx[] dialokasikan dinamis
 * ============================================================ */
int bfsTree(Node* start, Node* dest,
            Node** path, int* pathLen,
            Node** nodes, int nodeCount) {
    *pathLen = 0;
    if (!start || !dest) return 0;
    if (start == dest) {
        path[0] = start;
        *pathLen = 1;
        return 1;
    }

    /* Dynamic arrays untuk BFS */
    bool* visited = (bool*)calloc(nodeCount, sizeof(bool));
    int*  parentIdx = (int*)malloc(nodeCount * sizeof(int));
    if (!visited || !parentIdx) {
        free(visited); free(parentIdx);
        return 0;
    }

    Queue q;
    initQueue(&q);

    enqueue(&q, (void*)start);
    visited[start->idx] = true;
    parentIdx[start->idx] = -1;

    int found = 0;
    while (!isQueueEmpty(&q) && !found) {
        Node* curr = (Node*)dequeue(&q);

        /* Helper macro: proses 1 tetangga */
        #define PROCESS(NB) do { \
            if ((NB) && !visited[(NB)->idx]) { \
                visited[(NB)->idx] = true; \
                parentIdx[(NB)->idx] = curr->idx; \
                if ((NB) == dest) { \
                    /* Rekonstruksi path */ \
                    int* stack = (int*)malloc(nodeCount * sizeof(int)); \
                    int sp = 0, idx = dest->idx; \
                    while (idx != -1) { stack[sp++] = idx; idx = parentIdx[idx]; } \
                    for (int ri = 0; ri < sp; ri++) \
                        path[ri] = nodes[stack[sp - 1 - ri]]; \
                    *pathLen = sp; \
                    free(stack); \
                    found = 1; \
                } else { \
                    enqueue(&q, (void*)(NB)); \
                } \
            } \
        } while (0)

        /* Cari tetangga via PARENT (atas) */
        PROCESS(curr->parent);

        /* Cari tetangga via CHILDREN (bawah, firstSon chain) */
        Node* child = curr->firstSon;
        while (child && !found) {
            PROCESS(child);
            child = child->nextBrother;
        }

        #undef PROCESS
    }

    free(visited);
    free(parentIdx);
    freeQueue(&q);

    return found;
}
