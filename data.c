/*
 * DATA MODULE — Implementation
 * =============================
 * Modul ini mengelola SEMUA data program.
 *
 * TRAVERSAL YANG DIGUNAKAN:
 *   ┌─────────────────┬───────────────┬──────────────────────────────────────┐
 *   │ Fungsi          │ Traversal     │ Keterangan                           │
 *   ├─────────────────┼───────────────┼──────────────────────────────────────┤
 *   │ parseHalte()    │ Linear        │ Baca file baris per baris (O(n))     │
 *   │ initNodes()     │ Linear        │ Scan koridor, buat node unik (O(n))  │
 *   │ identifyTransit │ Linear        │ Scan koridor, set bitmask (O(n))     │
 *   │ buildEdgeMapping│ Linear        │ Scan koridor, mapping edge (O(n))    │
 *   └─────────────────┴───────────────┴──────────────────────────────────────┘
 *   buildTree() → buildNaryTree() di nbtree.c (DFS Pre-order)
 *
 * 1. PARSE & STORAGE
 *    parseHalte()     : baca halte.txt → koridors[] (dynamic array)
 *    simpan_data()    : tulis koridors[] → halte.txt
 *
 * 2. NODE LIFECYCLE
 *    initNodes()      : buat Node untuk tiap halte unik (malloc)
 *    freeNodesOnly()  : hapus node tapi KORIDOR aman (untuk rebuild)
 *    cleanupNodes()   : hapus SEMUA (node + koridor + edge)
 *
 * 3. TREE BUILDING
 *    buildTree()      : wrapper → buildNaryTree() di nbtree.c (DFS Pre-order)
 *    buildEdgeMapping(): mapping A->B ke nomor koridor (linear scan)
 *    identifyTransit(): tandai node yang muncul di ≥2 koridor (linear scan)
 *
 * 4. CRUD
 *    tambah_halte()   : tambah halte ke koridor + rebuild
 *    hapus_halte()    : hapus halte dari koridor + rebuild
 *
 * Semua array global dialokasikan dinamis (NULL awal, realloc saat penuh).
 * Kapasitas awal: nodes=16, edges=16, koridors=4 — digandakan tiap penuh.
 */
#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================
 * GLOBAL VARIABLES — semua dynamic (NULL awal, realloc saat perlu)
 * ============================================================ */
Node**   nodes           = NULL;
char**   nodeInitials    = NULL;
int      nodeCount       = 0;

Koridor* koridors        = NULL;
int      koridorCount    = 0;

char**   edgeKeys        = NULL;
int*     edgeValues      = NULL;
int      edgeCount       = 0;

int*     halteKoridorBits  = NULL;
int*     halteKoridorCount = NULL;
bool*    isTransit         = NULL;

/* ============================================================
 * HELPERS DINAMIS
 * ============================================================ */

static int nodeCapacity   = 0;
static int edgeCapacity   = 0;
static int koridorCapacity = 0;

static void growNodes(void) {
    if (nodeCount < nodeCapacity) return;
    nodeCapacity = nodeCapacity ? nodeCapacity * 2 : 16;
    nodes           = (Node**)realloc(nodes,           nodeCapacity * sizeof(Node*));
    nodeInitials    = (char**)realloc(nodeInitials,    nodeCapacity * sizeof(char*));
    halteKoridorBits  = (int*)realloc(halteKoridorBits,  nodeCapacity * sizeof(int));
    halteKoridorCount = (int*)realloc(halteKoridorCount, nodeCapacity * sizeof(int));
    isTransit         = (bool*)realloc(isTransit,         nodeCapacity * sizeof(bool));
    for (int i = nodeCount; i < nodeCapacity; i++) {
        nodeInitials[i] = NULL;
        halteKoridorBits[i] = 0;
        halteKoridorCount[i] = 0;
        isTransit[i] = false;
    }
}

static void growEdges(void) {
    if (edgeCount < edgeCapacity) return;
    edgeCapacity = edgeCapacity ? edgeCapacity * 2 : 16;
    edgeKeys   = (char**)realloc(edgeKeys,   edgeCapacity * sizeof(char*));
    edgeValues = (int*)realloc(edgeValues, edgeCapacity * sizeof(int));
}

static void addKoridor(void) {
    if (koridorCount >= koridorCapacity) {
        koridorCapacity = koridorCapacity ? koridorCapacity * 2 : 4;
        koridors = (Koridor*)realloc(koridors, koridorCapacity * sizeof(Koridor));
    }
    koridors[koridorCount].names    = NULL;
    koridors[koridorCount].size     = 0;
    koridors[koridorCount].capacity = 0;
    koridorCount++;
}

static void addHalteToKoridor(Koridor* kor, const char* name) {
    if (kor->size >= kor->capacity) {
        kor->capacity = kor->capacity ? kor->capacity * 2 : 8;
        kor->names = (char**)realloc(kor->names, kor->capacity * sizeof(char*));
    }
    kor->names[kor->size] = (char*)malloc(strlen(name) + 1);
    strcpy(kor->names[kor->size], name);
    kor->size++;
}

/* ============================================================
 * trim
 * ============================================================ */
char* trim(char* s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char* end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return s;
}

/* ============================================================
 * makeInitials
 * ============================================================ */
void makeInitials(const char* name, char* out) {
    char temp[MAX_NAME];
    strcpy(temp, name);
    out[0] = '\0';
    char* token = strtok(temp, " ");
    while (token != NULL) {
        char buf[2] = {toupper((unsigned char)token[0]), '\0'};
        strcat(out, buf);
        token = strtok(NULL, " ");
    }
}

/* ============================================================
 * resolveDuplicates
 * ============================================================ */
void resolveDuplicates(void) {
    for (int i = 0; i < nodeCount; i++) {
        for (int j = i + 1; j < nodeCount; j++) {
            if (strcmp(nodeInitials[i], nodeInitials[j]) == 0) {
                char buf[MAX_NAME + 4];
                snprintf(buf, sizeof(buf), "%s2", nodeInitials[j]);
                free(nodeInitials[j]);
                nodeInitials[j] = (char*)malloc(strlen(buf) + 1);
                strcpy(nodeInitials[j], buf);
                snprintf(buf, sizeof(buf), "%s1", nodeInitials[i]);
                free(nodeInitials[i]);
                nodeInitials[i] = (char*)malloc(strlen(buf) + 1);
                strcpy(nodeInitials[i], buf);
            }
        }
    }
}

/* ============================================================
 * simpan_data - Tulis data koridor ke file
 * ============================================================ */
void simpan_data(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) { printf("Error: Gagal menulis %s\n", filename); return; }
    for (int k = 0; k < koridorCount; k++) {
        fprintf(file, "#Koridor %d\n", k + 1);
        for (int i = 0; i < koridors[k].size; i++)
            fprintf(file, "%s\n", koridors[k].names[i]);
    }
    fclose(file);
    printf("Data tersimpan ke %s\n", filename);
}

/* ============================================================
 * addEdge / findEdgeValue
 * ============================================================ */
void addEdge(const char* key, int value) {
    for (int i = 0; i < edgeCount; i++)
        if (strcmp(edgeKeys[i], key) == 0) return;
    growEdges();
    int idx = edgeCount++;
    edgeKeys[idx] = (char*)malloc(strlen(key) + 1);
    strcpy(edgeKeys[idx], key);
    edgeValues[idx] = value;
}

int findEdgeValue(const char* key) {
    for (int i = 0; i < edgeCount; i++)
        if (strcmp(edgeKeys[i], key) == 0) return edgeValues[i];
    return 0;
}

/* ============================================================
 * parseHalte
 * ============================================================ */
void parseHalte(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) { printf("Error: Gagal membuka %s\n", filename); exit(1); }

    char line[MAX_NAME];
    while (fgets(line, sizeof(line), file)) {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        char* t = trim(line);
        if (t[0] == '\0') continue;
        if (t[0] == '#') {
            addKoridor();
        } else if (koridorCount > 0) {
            addHalteToKoridor(&koridors[koridorCount - 1], t);
        }
    }
    fclose(file);
}

/* ============================================================
 * buildEdgeMapping
 * ============================================================ */
void buildEdgeMapping(void) {
    for (int k = 0; k < koridorCount; k++) {
        for (int i = 0; i < koridors[k].size - 1; i++) {
            char k1[MAX_KEY], k2[MAX_KEY];
            snprintf(k1, sizeof(k1), "%s->%s",
                     koridors[k].names[i], koridors[k].names[i + 1]);
            snprintf(k2, sizeof(k2), "%s->%s",
                     koridors[k].names[i + 1], koridors[k].names[i]);
            addEdge(k1, k + 1);
            addEdge(k2, k + 1);
        }
    }
}

/* ============================================================
 * identifyTransit
 * ============================================================ */
void identifyTransit(void) {
    for (int i = 0; i < nodeCount; i++) {
        halteKoridorBits[i] = 0;
        halteKoridorCount[i] = 0;
        isTransit[i] = false;
    }
    for (int k = 0; k < koridorCount; k++)
        for (int i = 0; i < koridors[k].size; i++) {
            int idx = findNodeIndex(nodes, nodeCount, koridors[k].names[i]);
            if (idx < 0) continue;
            if (!((halteKoridorBits[idx] >> k) & 1)) {
                halteKoridorBits[idx] |= (1 << k);
                halteKoridorCount[idx]++;
            }
        }
    for (int i = 0; i < nodeCount; i++)
        if (halteKoridorCount[i] >= 2) isTransit[i] = true;
}

/* ============================================================
 * tambah_halte - Tambah halte ke koridor
 * Return: 1 sukses, 0 duplikat, -1 error
 * ============================================================ */
int tambah_halte(const char* nama, int koridor) {
    if (findNodeIndex(nodes, nodeCount, nama) >= 0) return 0;
    if (koridor < 0 || koridor >= koridorCount) return -1;

    Koridor* korp = &koridors[koridor];
    if (korp->size >= korp->capacity) {
        korp->capacity = korp->capacity ? korp->capacity * 2 : 8;
        korp->names = (char**)realloc(korp->names, korp->capacity * sizeof(char*));
        if (!korp->names) return -1;
    }
    korp->names[korp->size] = (char*)malloc(strlen(nama) + 1);
    if (!korp->names[korp->size]) return -1;
    strcpy(korp->names[korp->size], nama);
    korp->size++;
    return 1;
}

/* ============================================================
 * hapus_halte - Hapus halte dari koridor
 * Return: 1 sukses, -1 error
 * ============================================================ */
int hapus_halte(int koridor, int index) {
    if (koridor < 0 || koridor >= koridorCount) return -1;
    if (index < 0 || index >= koridors[koridor].size) return -1;

    free(koridors[koridor].names[index]);
    for (int i = index; i < koridors[koridor].size - 1; i++)
        koridors[koridor].names[i] = koridors[koridor].names[i + 1];
    koridors[koridor].size--;
    return 1;
}

/* ============================================================
 * freeNodesOnly - Bebaskan node & tree, tapi KORIDOR & EDGE aman
 * ============================================================ */
void freeNodesOnly(void) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i])        freeNode(nodes[i]);
        if (nodeInitials[i]) free(nodeInitials[i]);
    }
    free(nodes);          nodes          = NULL;
    free(nodeInitials);   nodeInitials   = NULL;
    free(halteKoridorBits);  halteKoridorBits  = NULL;
    free(halteKoridorCount); halteKoridorCount = NULL;
    free(isTransit);         isTransit         = NULL;
    nodeCapacity = 0;
    nodeCount    = 0;
}

/* ============================================================
 * initNodes
 * ============================================================ */
void initNodes(void) {
    for (int k = 0; k < koridorCount; k++) {
        for (int i = 0; i < koridors[k].size; i++) {
            if (findNodeIndex(nodes, nodeCount, koridors[k].names[i]) >= 0)
                continue;
            growNodes();
            int idx = nodeCount++;
            nodes[idx] = createNode(koridors[k].names[i], idx);
            nodeInitials[idx] = (char*)malloc(MAX_NAME);
            makeInitials(koridors[k].names[i], nodeInitials[idx]);
        }
    }
    resolveDuplicates();
}

/* ============================================================
 * getRoot
 * ============================================================ */
Node* getRoot(void) {
    if (koridorCount > 0 && koridors[0].size > 0) {
        int ri = findNodeIndex(nodes, nodeCount, koridors[0].names[0]);
        if (ri >= 0) return nodes[ri];
    }
    return NULL;
}

/* ============================================================
 * buildTree
 * ============================================================ */
void buildTree(Node* root) {
    if (!root) return;
    bool* visited = (bool*)calloc(nodeCount, sizeof(bool));
    buildNaryTree(root, visited, koridors, koridorCount, nodes, nodeCount);
    free(visited);
}

/* ============================================================
 * cleanupNodes - Bebaskan SEMUA memori dinamis
 * ============================================================ */
void cleanupNodes(void) {
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i])    freeNode(nodes[i]);
        if (nodeInitials[i]) free(nodeInitials[i]);
    }
    free(nodes);    nodes    = NULL;
    free(nodeInitials); nodeInitials = NULL;
    free(halteKoridorBits);  halteKoridorBits  = NULL;
    free(halteKoridorCount); halteKoridorCount = NULL;
    free(isTransit);         isTransit         = NULL;

    for (int k = 0; k < koridorCount; k++) {
        for (int i = 0; i < koridors[k].size; i++)
            free(koridors[k].names[i]);
        free(koridors[k].names);
    }
    free(koridors); koridors = NULL;
    koridorCount = 0;
    nodeCount = 0;

    for (int i = 0; i < edgeCount; i++)
        free(edgeKeys[i]);
    free(edgeKeys);   edgeKeys   = NULL;
    free(edgeValues); edgeValues = NULL;
    edgeCount = 0;
}
