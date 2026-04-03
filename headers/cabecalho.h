#ifndef CABECALHO_H
#define CABECALHO_H

#include <stdio.h>
#include <stdbool.h>
#define TAM_CABECALHO 17

typedef struct Cabecalho {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
} Cabecalho;

void inicializarCabecalho(FILE *file);
bool lerStatusCabecalho(const char *nomeArquivo, char *statusOut);
void atualizarStatus(FILE *file, char status, bool seek);
void atualizarProxRRN(FILE *file, int proxRRN, bool seek);
void atualizarNroEstacoes(FILE *file, int nroEstacoes, bool seek);
void atualizarNroParesEstacoes(FILE *file, int nroParesEstacao, bool seek);
void recalcularContadores(FILE *file);

#endif