#include "../../headers/operacoes/juncoes.h"
#include "../../headers/operacoes/ordenacoes.h"
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

void handleSortMergeJoin() {
    char *arquivoDados1 = lerNomeArquivo();
    char *campo1 = lerNomeArquivo();
    char *arquivoDados2 = lerNomeArquivo();
    char *campo2 = lerNomeArquivo();

    if (!arquivoDados1 || !arquivoDados2 || !campo1 || !campo2) {
        printf("Falha no processamento do arquivo.\n");
        if (arquivoDados1) free(arquivoDados1);
        if (arquivoDados2) free(arquivoDados2);
        if (campo1) free(campo1);
        if (campo2) free(campo2);
        return;
    }

    int qtd1 = 0, qtd2 = 0;

    Registro *vetor1 = ordenarNaMemoria(arquivoDados1, campo1, &qtd1, NULL, NULL); // campo 1 é codProxEstacao
    Registro *vetor2 = ordenarNaMemoria(arquivoDados2, campo2, &qtd2, NULL, NULL); // campo 2 é codEstacao

    // limpeza de alocação, em caso de falha de alocação em um dos vetores
    if (!vetor1 || !vetor2) {
        for (int i = 0; i < qtd1; i++) {
            if (vetor1[i].tamNomeEstacao > 0) free(vetor1[i].nomeEstacao);
            if (vetor1[i].tamNomeLinha > 0) free(vetor1[i].nomeLinha);
        }
        free(vetor1);

        for (int i = 0; i < qtd2; i++) {
            if (vetor2[i].tamNomeEstacao > 0) free(vetor2[i].nomeEstacao);
            if (vetor2[i].tamNomeLinha > 0) free(vetor2[i].nomeLinha);
        }
        free(vetor2);

        free(arquivoDados1); free(arquivoDados2); free(campo1); free(campo2);
        return;
    }

    int p1 = 0, p2 = 0; // p1: ponteiro para vetor1, p2: ponteiro para vetor2
    bool encontrou = false;

    // loop de mesclagem
    while (p1 < qtd1 && p2 < qtd2) { // comparação na base do campo de ordenação escolhido para cada vetor
        int val1 = (strcmp(campo1, "codEstacao") == 0) ? vetor1[p1].codEstacao : vetor1[p1].codProxEstacao;
        int val2 = (strcmp(campo2, "codEstacao") == 0) ? vetor2[p2].codEstacao : vetor2[p2].codProxEstacao;

        // valores nulos não se cruzam e devem ir para o final do arquivo
        if (val1 == -1) { 
            p1++; 
            continue; 
        }

        if (val2 == -1) { 
            p2++; 
            continue; 
        }

        if (val1 < val2) p1++; // ponteiro do vetor 1 avança, buscando um valor igual ou maior ao do vetor 2

        else if (val1 > val2) p2++; // mesma lógica, mas para o vetor 2

        else {
            encontrou = true;
            int p2_temp = p2;
            
            // múltiplos "matches" sem perder a referência original a p2 (relação N:M)
            while (p2_temp < qtd2) {
                int val2_temp = (strcmp(campo2, "codEstacao") == 0) ? vetor2[p2_temp].codEstacao : vetor2[p2_temp].codProxEstacao;
                if (val2_temp != val1) break;

                // formatação nos valores codEstacao, nomeEstacao, nomeLinha, codProxEstacao e nomeEstacao do segundo arquivo
                printf("%d ", vetor1[p1].codEstacao);
                printf("%s ", vetor1[p1].nomeEstacao);
                
                if (vetor1[p1].tamNomeLinha > 0) printf("%s ", vetor1[p1].nomeLinha);
                else printf("NULO ");
                
                if (vetor1[p1].codProxEstacao != -1) printf("%d ", vetor1[p1].codProxEstacao);
                else printf("NULO ");
                
                if (vetor2[p2_temp].tamNomeEstacao > 0) printf("%s\n", vetor2[p2_temp].nomeEstacao);
                else printf("NULO\n");
                
                p2_temp++;
            }
            p1++; // avança apenas p1, permitindo que o próximo p1 verifique a mesma base do p2, caso seja necessário
        }
    }

    if (!encontrou) {
        printf("Registro inexistente.\n");
    }

    // liberação de memória
    for (int i = 0; i < qtd1; i++) {
        if (vetor1[i].tamNomeEstacao > 0) free(vetor1[i].nomeEstacao);
        if (vetor1[i].tamNomeLinha > 0) free(vetor1[i].nomeLinha);
    }
    free(vetor1);

    for (int i = 0; i < qtd2; i++) {
        if (vetor2[i].tamNomeEstacao > 0) free(vetor2[i].nomeEstacao);
        if (vetor2[i].tamNomeLinha > 0) free(vetor2[i].nomeLinha);
    }
    free(vetor2);

    free(arquivoDados1);
    free(arquivoDados2);
    free(campo1);
    free(campo2);

    return;
}