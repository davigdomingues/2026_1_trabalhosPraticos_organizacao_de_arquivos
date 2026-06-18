#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/utils.h"
#include "../../headers/fornecidas.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleOrderBy(){
    char *arquivoDados = NULL;
    FILE *fileDados = NULL;

    arquivoDados = lerNomeArquivo();
    if (!arquivoDados) return;

    fileDados = fopen(arquivoDados, "rb");
    if (!fileDados) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados);
        return;
    }
}