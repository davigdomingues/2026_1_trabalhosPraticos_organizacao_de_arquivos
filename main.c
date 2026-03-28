#include "operacoes.h"
#include "cabecalho.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    int op;
    scanf("%d", &op);

    char *arquivoEntrada = NULL;
    char *arquivoSaida = NULL;
    switch (op) {
        case 1:
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            arquivoSaida = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);
            scanf("%s", arquivoSaida);

            create(arquivoEntrada, arquivoSaida);
            break;
        case 2:
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);

            selectAll(arquivoEntrada);
            break;
        default:
            return -1;
    }
    free(arquivoEntrada);
    free(arquivoSaida);

    return 0;
}