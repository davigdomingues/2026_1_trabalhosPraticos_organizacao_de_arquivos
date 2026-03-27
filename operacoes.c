#include "operacoes.h"
#include "registro.h"
#include "cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-hashmap/map.h" //usando uma biblioteca, por enquanto. créditos da biblioteca para Mashpoe.

int freeMapKeys(const void* key, size_t ksize, uintptr_t value, void* usr){
    free((void*) key);
    return 0;
}

void create(){
    FILE *file = fopen("arquivo", "wb");
    inicializarCabecalho(file);

    int proxRRN = 0;
    //cria um hashmap para depois obter, eficientemente, o nroEstacoes únicas
    hashmap *mapEstacoes = hashmap_create();
    //cria um hashmap para depois obter, eficientemente, o nroParesEstacoes únicas
    hashmap *mapParesEstacoes = hashmap_create();

    FILE *csv = fopen("estacoes.csv", "r");
    char *linha = (char*) malloc(105 * sizeof(char));
    fgets(linha, 105, csv); //ignora linha de nomes das colunas
    while(fgets(linha, 105, csv) != NULL){
        char *linhaPtr = linha;

        //campos que garantidamente não são nulos
        int codEstacao = atoi(strsep(&linhaPtr, ","));
        char *nomeEstacao = strsep(&linhaPtr, ",");

        Registro *reg = (Registro*) malloc(sizeof(Registro));
        //inicializa a struct com valores que não precisam de tratamento
        *reg = (Registro) {.removido = '0', .proximo = -1, .codEstacao = codEstacao, .tamNomeEstacao = strlen(nomeEstacao), .nomeEstacao = nomeEstacao};

        //campos possívelmente nulos
        char *codLinha = strsep(&linhaPtr, ",");
        char *nomeLinha = strsep(&linhaPtr, ",");
        char *codProxEst = strsep(&linhaPtr, ",");
        char *distanciaProxEst = strsep(&linhaPtr, ",");
        char *codLinhaIntegra = strsep(&linhaPtr, ",");
        char *codEstacaoIntegra = strsep(&linhaPtr, "\n\r"); //por ser o último da linha seus delimitadores são diferentes

        //se o campo for nulo, põe -1 como valor
        reg->codLinha = *codLinha ? atoi(codLinha) : -1;
        reg->codProxEstacao = *codProxEst ? atoi(codProxEst) : -1;
        reg->distanciaProxEstacao = *distanciaProxEst ? atoi(distanciaProxEst) : -1;
        reg->codLinhaIntegra = *codLinhaIntegra ? atoi(codLinhaIntegra) : -1;
        reg->codEstIntegra = *codEstacaoIntegra ? atoi(codEstacaoIntegra) : -1;
        
        //se o nomeLinha for nulo, põe NULL como valor e põe o tamanho como 0
        if(*nomeLinha != '\0'){
            reg->nomeLinha = nomeLinha;
            reg->tamNomeLinha = strlen(nomeLinha);
        } else {
            reg->nomeLinha = NULL;
            reg->tamNomeLinha = 0; 
        }
        escreverReg(file, reg);

        //salva no hashmap com o nome da estação sendo a chave, para garantir unicidade
        //o valor salvo não importa
        hashmap_set(mapEstacoes, strdup(reg->nomeEstacao), reg->tamNomeEstacao+1, reg->codEstacao);

        if(reg->codProxEstacao != -1){
            char *par = (char*) malloc(sizeof(char) * 10);
            //constrói uma string para representar unicamente o par
            snprintf(par, 10, "%d-%d", reg->codEstacao, reg->codProxEstacao);
            //salva no hashmap. o valor salvo não importa
            hashmap_set(mapParesEstacoes, par, 10, reg->codProxEstacao);
        }

        proxRRN++;
    }

    atualizarStatus(file, '1', true);
    atualizarProxRRN(file, proxRRN, true);
    atualizarNroEstacoes(file, hashmap_size(mapEstacoes), false); //false porque nroEstacoes é o campo seguinte de proxRRN
    atualizarNroParesEstacoes(file, hashmap_size(mapParesEstacoes), false); //false porque nroParesEstacoes é o campo seguinte de nroEstacoes

    hashmap_iterate(mapEstacoes, freeMapKeys, NULL);
    hashmap_free(mapEstacoes);
    hashmap_iterate(mapParesEstacoes, freeMapKeys, NULL);
    hashmap_free(mapParesEstacoes);

    fclose(csv);
    fclose(file);
}

void selectAll(){
    FILE *file = fopen("arquivo", "rb");
    fseek(file, TAM_CABECALHO, SEEK_SET); 

    char removido;
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        fseek(file, 4, SEEK_CUR); //pula os 4 bytes de proxRRN

        Registro *reg = (Registro*) malloc(sizeof(Registro));

        //lê os campos do registro e armazena na struct
        fread(&reg->codEstacao, sizeof(int), 1, file);
        fread(&reg->codLinha, sizeof(int), 1, file);
        fread(&reg->codProxEstacao, sizeof(int), 1, file);
        fread(&reg->distanciaProxEstacao, sizeof(int), 1, file);
        fread(&reg->codLinhaIntegra, sizeof(int), 1, file);
        fread(&reg->codEstIntegra, sizeof(int), 1, file);

        fread(&reg->tamNomeEstacao, sizeof(int), 1, file);
        if(reg->tamNomeEstacao != 0){
            char *nomeEstacao = (char*) malloc( sizeof(char) * reg->tamNomeEstacao);
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        }

        fread(&reg->tamNomeLinha, sizeof(int), 1, file);
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc(sizeof(char) * reg->tamNomeLinha);
            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, file);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;
        }

        printReg(reg);
        int tamRestante = TAM_REG - 9 * sizeof(int) - sizeof(char) - reg->tamNomeEstacao - reg->tamNomeLinha;
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        free(reg->nomeEstacao);
        free(reg->nomeLinha);
        free(reg);
    }
}