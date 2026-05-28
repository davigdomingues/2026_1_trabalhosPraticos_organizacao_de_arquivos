#ifndef BTREE_CABECALHO_H
#define BTREE_CABECALHO_H

#include <stdbool.h>
#define TAM_BTREE_CABECALHO 17

typedef struct CabecalhoIndice {
    int noRaiz;
    int topo;
    int proxRRN;
    int nroNos;
    char status;
} CabecalhoIndice;

#endif