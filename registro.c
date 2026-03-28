#include "registro.h"
#include <stdio.h>
#include <string.h>

void escreverReg(FILE *file, Registro *reg){
    fwrite(&reg->removido, sizeof(char), 1, file);
    fwrite(&reg->proximo, sizeof(int), 1, file);
    fwrite(&reg->codEstacao, sizeof(int), 1, file);
    fwrite(&reg->codLinha, sizeof(int), 1, file);
    fwrite(&reg->codProxEstacao, sizeof(int), 1, file);
    fwrite(&reg->distProxEstacao, sizeof(int), 1, file);
    fwrite(&reg->codLinhaIntegra, sizeof(int), 1, file);
    fwrite(&reg->codEstIntegra, sizeof(int), 1, file);
    fwrite(&reg->tamNomeEstacao, sizeof(int), 1, file);
    fwrite(reg->nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
    fwrite(&reg->tamNomeLinha, sizeof(int), 1, file);
    fwrite(reg->nomeLinha, sizeof(char), reg->tamNomeLinha, file);

    //subtrai do tamanho total o tamanho dos campos fixos e o tamanho dos campos variáveis
    int tamRestante = TAM_REG - 9 * sizeof(int) - sizeof(char ) - reg->tamNomeEstacao - reg->tamNomeLinha;
    char filler[tamRestante];
    //preenche o restante do registro com '$'
    memset(filler, '$', tamRestante);
    fwrite(filler, sizeof(char), tamRestante, file);
}

void printReg(Registro *reg){
    //campos não nulos
    printf("%d ", reg->codEstacao);
    printf("%s ", reg->nomeEstacao);

    //campos possívelmente nulos
    if(reg->codLinha != -1) printf("%d ", reg->codLinha);
    else printf("%s ", "NULO");

    if(reg->tamNomeLinha != 0) printf("%s ", reg->nomeLinha);
    else printf("%s ", "NULO");

    if(reg->codProxEstacao != -1) printf("%d ", reg->codProxEstacao);
    else printf("%s ", "NULO");

    if(reg->distProxEstacao != -1) printf("%d ", reg->distProxEstacao);
    else printf("%s ", "NULO");

    if(reg->codLinhaIntegra != -1) printf("%d ", reg->codLinhaIntegra);
    else printf("%s ", "NULO");

    if(reg->codEstIntegra != -1) printf("%d", reg->codEstIntegra);
    else printf("%s", "NULO");

    printf("\n");
}