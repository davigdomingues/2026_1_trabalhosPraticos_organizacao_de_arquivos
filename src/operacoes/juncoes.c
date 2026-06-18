#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/utils.h"
#include "../../headers/fornecidas.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleNestedJoin(){
    char *arquivoDados1 = NULL;
    char *arquivoDados2 = NULL;
    FILE *fileDados1 = NULL;
    FILE *fileDados2 = NULL;

    arquivoDados1 = lerNomeArquivo();
    if (!arquivoDados1) return;

    arquivoDados2 = lerNomeArquivo();
    if (!arquivoDados2) {
        free(arquivoDados1);
        return;
    }

    fileDados1 = fopen(arquivoDados1, "rb");
    if (!fileDados1) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }

    fileDados2 = fopen(arquivoDados2, "rb");
    if (!fileDados2) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados1);
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }
}

void handleIndexedJoin(){
    char *arquivoDados1 = NULL;
    char *arquivoDados2 = NULL;
    FILE *fileDados1 = NULL;
    FILE *fileDados2 = NULL;

    arquivoDados1 = lerNomeArquivo();
    if (!arquivoDados1) return;

    arquivoDados2 = lerNomeArquivo();
    if (!arquivoDados2) {
        free(arquivoDados1);
        return;
    }

    fileDados1 = fopen(arquivoDados1, "rb");
    if (!fileDados1) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }

    fileDados2 = fopen(arquivoDados2, "rb");
    if (!fileDados2) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados1);
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }
}

void handleSortMergeJoin(){
    char *arquivoDados1 = NULL;
    char *arquivoDados2 = NULL;
    FILE *fileDados1 = NULL;
    FILE *fileDados2 = NULL;

    arquivoDados1 = lerNomeArquivo();
    if (!arquivoDados1) return;

    arquivoDados2 = lerNomeArquivo();
    if (!arquivoDados2) {
        free(arquivoDados1);
        return;
    }

    fileDados1 = fopen(arquivoDados1, "rb");
    if (!fileDados1) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }

    fileDados2 = fopen(arquivoDados2, "rb");
    if (!fileDados2) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados1);
        free(arquivoDados1);
        free(arquivoDados2);
        return;
    }
}