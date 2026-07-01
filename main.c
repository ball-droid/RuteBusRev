/*
 * MAIN MODULE — Entry Point & Orchestrator
 * =========================================
 * TRAVERSAL:
 *   menu_cari()     → BFS Level-order (bfsTree di nbtree)
 *   saveTreeToFile() → DFS Pre-order (printNodeRecursive)
 *   buildTree()     → DFS Pre-order (buildNaryTree di nbtree)
 *
 * ALUR:
 *   1. parseHalte  → rebuild  → menu loop
 *   2. [1] Cari Rute : bfsTree → display → saveTreeToFile
 *   3. [2] Tambah    : tambah_halte → rebuild → simpan_data
 *   4. [3] Hapus     : hapus_halte → rebuild → simpan_data
 *   5. [4] Keluar    : kosongkan tree.txt → cleanupNodes → exit
 */
#include "data.h"
#include "nbtree.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Node* root;

static void rebuild(void) {
  freeNodesOnly();
  initNodes();
  root = getRoot();
  buildEdgeMapping();
  buildTree(root);
  identifyTransit();
}

/* ── INPUT ── */
static int getUserRoute(char* start, char* dest) {
  printf("  Halte keberangkatan: ");
  if (fgets(start, MAX_NAME, stdin)) {
    char* t = trim(start);
    memmove(start, t, strlen(t) + 1);
  }
  printf("  Halte tujuan       : ");
  if (fgets(dest, MAX_NAME, stdin)) {
    char* t = trim(dest);
    memmove(dest, t, strlen(t) + 1);
  }
  if (start[0] == '\0' || dest[0] == '\0') {
    printf("\nError: Nama halte tidak boleh kosong!\n");
    return 0;
  }
  int si = findNodeIndex(nodes, nodeCount, start);
  int di = findNodeIndex(nodes, nodeCount, dest);
  if (si < 0) {
    printf("\nError: '%s' tidak ditemukan!\n", start);
    return 0;
  }
  if (di < 0) {
    printf("\nError: '%s' tidak ditemukan!\n", dest);
    return 0;
  }
  return 1;
}

/* ── DISPLAY ── */
static void displayRouteStops(Node** path, int pathLen) {
  int cur = 0;
  for (int i = 0; i < pathLen; i++) {
    if (i > 0) {
      char ek[MAX_KEY];
      snprintf(ek, sizeof(ek), "%s->%s", path[i - 1]->name, path[i]->name);
      int kor = findEdgeValue(ek);
      if (kor && kor != cur) {
        printf("   Pindah ke Koridor %d\n", kor);
        cur = kor;
      }
    } else if (pathLen > 1) {
      char ek[MAX_KEY];
      snprintf(ek, sizeof(ek), "%s->%s", path[0]->name, path[1]->name);
      cur = findEdgeValue(ek);
    }
    printf("  %d. %s", i + 1, path[i]->name);
    if (i == 0) {
      printf(" (berangkat)%s", cur > 0 ? " - Koridor " : "");
    } else if (i == pathLen - 1) {
      printf(" (tujuan)");
    } else if (isTransit[path[i]->idx] && i > 0 && i < pathLen - 1) {
      char ek1[MAX_KEY], ek2[MAX_KEY];
      snprintf(ek1, sizeof(ek1), "%s->%s", path[i-1]->name, path[i]->name);
      snprintf(ek2, sizeof(ek2), "%s->%s", path[i]->name, path[i+1]->name);
      int k1 = findEdgeValue(ek1);
      int k2 = findEdgeValue(ek2);
      if (k1 && k2 && k1 != k2) {
        printf(" (transit)");
      }
    }
    if (cur > 0 && i == 0 && pathLen > 1) {
      printf("%d", cur);
    }
    printf("\n");
  }
}

static void displayTransitDetails(Node** path, int pathLen) {
  int tn = 0;
  for (int i = 0; i < pathLen; i++) {
    if (!isTransit[path[i]->idx] || i == 0 || i == pathLen - 1) {
      continue;
    }
    char p[MAX_KEY], n[MAX_KEY];
    snprintf(p, sizeof(p), "%s->%s", path[i-1]->name, path[i]->name);
    snprintf(n, sizeof(n), "%s->%s", path[i]->name, path[i+1]->name);
    int kp = findEdgeValue(p);
    int kn = findEdgeValue(n);
    if (kp && kn && kp != kn) {
      tn++;
      printf("  Transit %d: %s\n", tn, path[i]->name);
      printf("    Koridor %d -> Koridor %d\n", kp, kn);
    }
  }
  if (tn == 0) {
    printf("  Rute langsung, tanpa transit.\n");
  }
}

/* ── SAVE TREE ── */
static void saveTreeToFile(const char* filename, Node* root) {
  FILE* file = fopen(filename, "w");
  if (!file) { return; }
  if (root != NULL) {
    fprintf(file, "%s (root)\n", nodeInitials[root->idx]);
    int tc = 0;
    Node* tmp = root->firstSon;
    while (tmp) {
      tc++;
      tmp = tmp->nextBrother;
    }
    int cnt = 0;
    Node* ch = root->firstSon;
    while (ch) {
      cnt++;
      printNodeRecursive(file, ch, "", cnt == tc, nodeInitials);
      ch = ch->nextBrother;
    }
    fprintf(file, "\n");
  }
  fclose(file);
}

/* ── MENU 1: Cari Rute ── */
static void menu_cari(void) {
  char start[MAX_NAME], dest[MAX_NAME];
  printf("\nCari rute\n");
  if (!getUserRoute(start, dest)) { return; }
  printf("Mencari rute...\n");

  Node** path = (Node**)malloc(nodeCount * sizeof(Node*));
  int plen = 0;
  int si = findNodeIndex(nodes, nodeCount, start);
  int di = findNodeIndex(nodes, nodeCount, dest);

  if (!bfsTree(nodes[si], nodes[di], path, &plen, nodes, nodeCount)) {
    printf("Tidak ada rute dari '%s' ke '%s'.\n", start, dest);
    free(path);
    return;
  }
  printf("  Rute ditemukan!\n\n");
  printf("Rute: %s -> %s\n", path[0]->name, path[plen - 1]->name);
  printf("Jarak: %d ruas (%d halte)\n\n", plen - 1, plen);
  displayRouteStops(path, plen);
  displayTransitDetails(path, plen);
  free(path);
  saveTreeToFile("data/tree.txt", root);
}

/* ── MENU 2: Tambah Halte ── */
static void menu_tambah(void) {
  printf("\nTambah halte\n");
  for (int k = 0; k < koridorCount; k++) {
    printf("  %d. Koridor %d (%d halte)\n", k + 1, k + 1, koridors[k].size);
  }
  printf("Pilih koridor (1-%d): ", koridorCount);
  char buf[10];
  if (!fgets(buf, sizeof(buf), stdin)) { return; }
  int kor = atoi(buf) - 1;
  if (kor < 0 || kor >= koridorCount) {
    printf("Koridor tidak valid!\n");
    return;
  }

  printf("Masukkan nama halte baru: ");
  char nama[MAX_NAME];
  if (!fgets(nama, sizeof(nama), stdin)) { return; }
  {
    char* t = trim(nama);
    memmove(nama, t, strlen(t) + 1);
  }
  if (nama[0] == '\0') {
    printf("Nama tidak boleh kosong!\n");
    return;
  }

  int hasil = tambah_halte(nama, kor);
  if (hasil == 0) {
    printf("Halte '%s' sudah ada!\n", nama);
    return;
  }
  if (hasil < 0) {
    printf("Gagal menambah halte.\n");
    return;
  }
  rebuild();
  simpan_data("data/halte.txt");
  printf("Halte '%s' ditambahkan ke Koridor %d.\n", nama, kor + 1);
}

/* ── MENU 3: Hapus Halte ── */
static void menu_hapus(void) {
  printf("\nHapus halte\n");
  for (int k = 0; k < koridorCount; k++) {
    printf("\nKoridor %d (%d halte):\n", k + 1, koridors[k].size);
    for (int i = 0; i < koridors[k].size; i++) {
      printf("  %d. %s\n", i + 1, koridors[k].names[i]);
    }
  }
  printf("\nPilih koridor (1-%d): ", koridorCount);
  char buf[10];
  if (!fgets(buf, sizeof(buf), stdin)) { return; }
  int kor = atoi(buf) - 1;
  if (kor < 0 || kor >= koridorCount) {
    printf("Koridor tidak valid!\n");
    return;
  }
  if (koridors[kor].size == 0) {
    printf("Tidak ada halte.\n");
    return;
  }
  printf("Pilih nomor halte (1-%d): ", koridors[kor].size);
  if (!fgets(buf, sizeof(buf), stdin)) { return; }
  int idx = atoi(buf) - 1;
  if (idx < 0 || idx >= koridors[kor].size) {
    printf("Nomor tidak valid!\n");
    return;
  }
  printf("Hapus '%s'? (y/n): ", koridors[kor].names[idx]);
  if (!fgets(buf, sizeof(buf), stdin)) { return; }
  if (buf[0] != 'y' && buf[0] != 'Y') {
    printf("Dibatalkan.\n");
    return;
  }
  if (hapus_halte(kor, idx) < 0) {
    printf("Gagal menghapus.\n");
    return;
  }
  rebuild();
  simpan_data("data/halte.txt");
  printf("Halte berhasil dihapus.\n");
}

/* ── KELUAR ── */
static void keluar(void) {
  printf("Terima kasih! Sampai jumpa.\n");
  FILE* f = fopen("data/tree.txt", "w");
  if (f) { fclose(f); }
  cleanupNodes();
  exit(0);
}

/* ── MAIN ── */
int main(void) {
  parseHalte("data/halte.txt");
  rebuild();
  char buf[10];

  while (1) {
    printf("\n1. Cari Rute\n");
    printf("2. Tambah Halte\n");
    printf("3. Hapus Halte\n");
    printf("4. Keluar\n");
    printf("Pilihan: ");
    if (!fgets(buf, sizeof(buf), stdin)) { break; }
    switch (atoi(buf)) {
      case 1: {
        menu_cari();
        break;
      }
      case 2: {
        menu_tambah();
        break;
      }
      case 3: {
        menu_hapus();
        break;
      }
      case 4: {
        keluar();
        break;
      }
      default: {
        printf("Pilihan tidak valid! Masukkan 1-4.\n");
        break;
      }
    }
  }
  return 0;
}
