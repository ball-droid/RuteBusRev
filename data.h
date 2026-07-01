/*
 * DATA MODULE — Header
 * =====================
 * Modul ini menyediakan seluruh data global program:
 *   - Node** nodes[]           : array of pointer ke semua node tree
 *   - char** nodeInitials[]    : inisial tiap node (PD, RIBP, dll)
 *   - Koridor* koridors[]      : data koridor hasil parsing file
 *   - char** edgeKeys[]        : mapping "A->B" ke nomor koridor
 *   - int* halteKoridorBits[]  : bitmask koridor untuk tiap node
 *   - bool* isTransit[]        : true jika node muncul di >= 2 koridor
 *
 * Modul ini tergantung pada: nbtree.h (untuk tipe Node, Koridor)
 * Modul ini digunakan oleh: main.c
 * (nbtree.c TIDAK langsung gunakan data.h — dia terima parameter)
 *
 * Alokasi SEMUA array bersifat dinamis (realloc saat penuh).
 * cleanupNodes() membebaskan seluruh memori.
 */
#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include "nbtree.h"

#define MAX_KEY (MAX_NAME * 2 + 4)

extern Node**   nodes;
extern char**   nodeInitials;
extern int      nodeCount;

extern Koridor* koridors;
extern int      koridorCount;

extern char**   edgeKeys;
extern int*     edgeValues;
extern int      edgeCount;

extern int*     halteKoridorBits;
extern int*     halteKoridorCount;
extern bool*    isTransit;

char* trim(char* s);
void  makeInitials(const char* name, char* out);
void  resolveDuplicates(void);
void  parseHalte(const char* filename);
void  initNodes(void);
Node* getRoot(void);
void  cleanupNodes(void);
void  freeNodesOnly(void);
void  buildTree(Node* root);
void  buildEdgeMapping(void);
void  identifyTransit(void);
void  addEdge(const char* key, int value);
int   findEdgeValue(const char* key);

/* CRUD halte */
int   tambah_halte(const char* nama, int koridor);
int   hapus_halte(int koridor, int index);
void  simpan_data(const char* filename);

#endif
