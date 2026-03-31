#include "cabecalho.h"
#include <stdio.h>

void inicializarCabecalho(FILE *file){
    Cabecalho cabecalho = {.status = '1', .topo = -1, .proxRRN = 0, .nroEstacoes = 0, .nroParesEstacao = 0};
    fwrite(&cabecalho.status, sizeof(char), 1, file);
    fwrite(&cabecalho.topo, sizeof(int), 1, file);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, file);
    fwrite(&cabecalho.nroEstacoes, sizeof(int), 1, file);
    fwrite(&cabecalho.nroParesEstacao, sizeof(int), 1, file);
}

void atualizarStatus(FILE *file, char status, bool seek){
    if(seek) fseek(file, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, file);
}

void atualizarProxRRN(FILE *file, int proxRRN, bool seek){
    if(seek) fseek(file, 5, SEEK_SET);
    fwrite(&proxRRN, sizeof(int), 1, file);
}

void atualizarNroEstacoes(FILE *file, int nroEstacoes, bool seek){
    if(seek) fseek(file, 9, SEEK_SET);
    fwrite(&nroEstacoes, sizeof(int), 1, file);
}

void atualizarNroParesEstacoes(FILE *file, int nroParesEstacao, bool seek){
    if(seek) fseek(file, 13, SEEK_SET);
    fwrite(&nroParesEstacao, sizeof(int), 1, file);
}