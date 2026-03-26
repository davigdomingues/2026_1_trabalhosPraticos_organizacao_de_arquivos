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