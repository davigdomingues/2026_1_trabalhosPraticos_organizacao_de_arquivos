#include "../headers/cabecalho.h"
#include "../c-hashmap/map.h"
#include "../headers/registro.h"
#include "../headers/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void inicializarCabecalho(FILE *file){
    Cabecalho cabecalho = {.status = '0', .topo = -1, .proxRRN = 0, .nroEstacoes = 0, .nroParesEstacao = 0};
    fwrite(&cabecalho.status, sizeof(char), 1, file);
    fwrite(&cabecalho.topo, sizeof(int), 1, file);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, file);
    fwrite(&cabecalho.nroEstacoes, sizeof(int), 1, file);
    fwrite(&cabecalho.nroParesEstacao, sizeof(int), 1, file);
}

bool lerStatusCabecalho(const char *nomeArquivo, char *statusOut) {
    FILE *f = fopen(nomeArquivo, "rb");
    if (!f) return false;
    char status;
    if (fread(&status, sizeof(char), 1, f) != 1) {
        fclose(f);
        return false;
    }
    fclose(f);
    *statusOut = status;
    return true;
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

// utilizada após remoções, para garantir que os contadores estejam corretos sem a necessidade de realmente excluir os registros do arquivo
void recalcularContadores(FILE *file) {
    // recria os hashmaps para contar as estações e pares de estações únicas, varrendo o arquivo e lendo os registros um por um
    hashmap *mapEstacoes = hashmap_create();
    hashmap *mapParesEstacoes = hashmap_create();

    long posOriginal = ftell(file); // salva onde o ponteiro estava

    fseek(file, TAM_CABECALHO, SEEK_SET); // pula o cabeçalho (17 bytes)
    char removido;
    
    while(fread(&removido, sizeof(char), 1, file)) {
        if (removido == '1') {
            fseek(file, TAM_REG - 1, SEEK_CUR); // pula o resto do registro removido
            continue;
        }
        
        int codEstacao, codProxEstacao, tamNomeEstacao, tamNomeLinha;
        
        fseek(file, 4, SEEK_CUR); // pula próximo
        fread(&codEstacao, sizeof(int), 1, file);
        fseek(file, 4, SEEK_CUR); // pula codLinha
        fread(&codProxEstacao, sizeof(int), 1, file);
        fseek(file, 12, SEEK_CUR); // pula distProxEstacao, codLinhaIntegra, codEstIntegra
        
        fread(&tamNomeEstacao, sizeof(int), 1, file);
        char *nomeEstacao = NULL;
        if (tamNomeEstacao > 0) {
            nomeEstacao = (char*) malloc((size_t)tamNomeEstacao + 1);
            fread(nomeEstacao, sizeof(char), tamNomeEstacao, file);
            nomeEstacao[tamNomeEstacao] = '\0';
            hashmap_set(mapEstacoes, nomeEstacao, tamNomeEstacao + 1, codEstacao);
        }
        
        fread(&tamNomeLinha, sizeof(int), 1, file);
        fseek(file, tamNomeLinha, SEEK_CUR); // pula o nomeLinha (não precisamos dele)
        
        if (codProxEstacao != -1) {
            char *par = (char*) malloc(20);
            snprintf(par, 20, "%d-%d", codEstacao, codProxEstacao);
            hashmap_set(mapParesEstacoes, par, strlen(par) + 1, codProxEstacao);
        }

        // pula o lixo ($) para ir para o próximo registro
        int tamRestante = TAM_LIVRE_REG(tamNomeEstacao, tamNomeLinha);
        if (tamRestante > 0) fseek(file, tamRestante, SEEK_CUR);
    }

    int nroEstacoes = hashmap_size(mapEstacoes);
    int nroPares = hashmap_size(mapParesEstacoes);

    // escreve os novos contadores ajustados no cabeçalho
    fseek(file, 9, SEEK_SET); // o byte offset 9 é exatamente onde começa o nroEstacoes
    fwrite(&nroEstacoes, sizeof(int), 1, file);
    fwrite(&nroPares, sizeof(int), 1, file);

    // libera a memória
    hashmap_iterate(mapEstacoes, freeMapKeys, NULL);
    hashmap_free(mapEstacoes);
    hashmap_iterate(mapParesEstacoes, freeMapKeys, NULL);
    hashmap_free(mapParesEstacoes);

    fseek(file, posOriginal, SEEK_SET); // devolve o ponteiro de arquivo para onde estava
}