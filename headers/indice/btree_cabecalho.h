#ifndef BTREE_CABECALHO_H
#define BTREE_CABECALHO_H

#include <stdio.h>
#include <stdbool.h>

#define TAM_BTREE_CABECALHO 17
/* deslocamentos fixos do registro de cabeçalho da árvore-B:
    status: 1 byte
    noRaiz: 4 bytes
    topo: 4 bytes
    proxRRN: 4 bytes
    nroNos: 4 bytes 
*/

#define BTREE_OFF_STATUS 0 // 1 byte para status
#define BTREE_OFF_NORAIZ 1 // 4 bytes para noRaiz
#define BTREE_OFF_TOPO 5 // 4 bytes para topo
#define BTREE_OFF_PROXRRN 9 // 4 bytes para proxRRN
#define BTREE_OFF_NRONOS 13 // 4 bytes para nroNos

typedef struct CabecalhoIndice {
    int noRaiz;
    int topo;
    int proxRRN;
    int nroNos;
    char status;
} CabecalhoIndice;

bool atualizarStatusIndice(FILE *fileIndice, char status);
bool escreverCabecalhoIndice(FILE *fileIndice, char status, int noRaiz, int topo, int proxRRN, int nroNos);

#endif