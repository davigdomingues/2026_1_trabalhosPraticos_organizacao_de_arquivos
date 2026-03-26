#include "operacoes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void insert(FILE *file, int codEstacao, char *nomeEstacao, char *codLinha, char *nomeLinha, char *codProxEst, char *distanciaProxEst, char *codLinhaIntegra, char *codEstacaoIntegra){
    Registro reg = {.removido = 0, .proximo = -1, .codEstacao = codEstacao, .tamNomeEstacao = strlen(nomeEstacao), .nomeEstacao = nomeEstacao};

    //se o campo for nulo, põe -1 como valor
    reg.codLinha = *codLinha ? atoi(codLinha) : -1;
    reg.codProxEstacao = *codProxEst ? atoi(codProxEst) : -1;
    reg.distanciaProxEstacao = *distanciaProxEst ? atoi(distanciaProxEst) : -1;
    reg.codLinhaIntegra = *codLinhaIntegra ? atoi(codLinhaIntegra) : -1;
    reg.codEstIntegra = *codEstacaoIntegra ? atoi(codEstacaoIntegra) : -1;
        
    //se o nomeLinha for nulo, põe NULL como valor e põe o tamanho como 0
    if(*nomeLinha != '\0'){
        reg.nomeLinha = nomeLinha;
        reg.tamNomeLinha = strlen(nomeLinha);
    } else {
        reg.nomeLinha = NULL;
        reg.tamNomeLinha = 0; 
    }

    fwrite(&reg.removido, sizeof(char), 1, file);
    fwrite(&reg.proximo, sizeof(int), 1, file);
    fwrite(&reg.codEstacao, sizeof(int), 1, file);
    fwrite(&reg.codLinha, sizeof(int), 1, file);
    fwrite(&reg.codProxEstacao, sizeof(int), 1, file);
    fwrite(&reg.distanciaProxEstacao, sizeof(int), 1, file);
    fwrite(&reg.codLinhaIntegra, sizeof(int), 1, file);
    fwrite(&reg.codEstIntegra, sizeof(int), 1, file);
    fwrite(&reg.tamNomeEstacao, sizeof(int), 1, file);
    fwrite(reg.nomeEstacao, sizeof(char), reg.tamNomeEstacao, file);
    fwrite(&reg.tamNomeLinha, sizeof(int), 1, file);
    fwrite(reg.nomeLinha, sizeof(char), reg.tamNomeLinha, file);

    //subtrai do tamanho total o tamanho dos campos fixos e o tamanho dos campos variáveis
    int tamRestante = TAM_REG - 9 * sizeof(int) - sizeof(char ) - reg.tamNomeEstacao - reg.tamNomeLinha;
    char filler[tamRestante];
    //preenche o restante do registro com '$'
    memset(filler, '$', tamRestante);
    fwrite(filler, sizeof(char), tamRestante, file);
}