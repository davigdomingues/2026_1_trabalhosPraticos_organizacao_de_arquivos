#include <stdio.h>
typedef struct Cabecalho {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
} Cabecalho;

void inicializarCabecalho(FILE *file);
void atualizarStatus(FILE *file, char status);