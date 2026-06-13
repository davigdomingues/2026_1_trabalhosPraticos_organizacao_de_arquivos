#include "../../headers/operacoes/criacoes.h"
#include "../../headers/operacoes/insercoes.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/utils.h"
#include "../../c-hashmap/map.h" //usando uma biblioteca, créditos para Mashpoe.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool create(char *arquivoEntrada, char *arquivoSaida){
    FILE *file = fopen(arquivoSaida, "wb");

    int proxRRN = 0;
    //cria um hashmap para depois obter, eficientemente, o nroEstacoes únicas
    hashmap *mapEstacoes = hashmap_create();
    //cria um hashmap para depois obter, eficientemente, o nroParesEstacoes únicas
    hashmap *mapParesEstacoes = hashmap_create();

    FILE *csv = fopen(arquivoEntrada, "r");
    if(!csv){
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return false;
    }

    inicializarCabecalho(file);

    char *linha = (char*) malloc(105 * sizeof(char));
    fgets(linha, 105, csv); //ignora linha de nomes das colunas
    while(fgets(linha, 105, csv) != NULL){
        char *linhaPtr = linha;

        //campos que garantidamente não são nulos
        int codEstacao = atoi(strsep(&linhaPtr, ","));
        char *nomeEstacao = strsep(&linhaPtr, ",");

        Registro *reg = (Registro*) malloc(sizeof(Registro));
        if(!reg){
            free(linha);
            fclose(csv);
            fclose(file);
            printf("Falha no processamento do arquivo.\n");
            return false;
        }

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
        reg->distProxEstacao = *distanciaProxEst ? atoi(distanciaProxEst) : -1;
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
            int menor = (reg->codEstacao < reg->codProxEstacao) ? reg->codEstacao : reg->codProxEstacao;
            int maior = (reg->codEstacao < reg->codProxEstacao) ? reg->codProxEstacao : reg->codEstacao;

            char *par = (char*) malloc(sizeof(char) * 10);
            //constrói uma string para representar o par unicamente
            snprintf(par, 10, "%d-%d", menor, maior);

            //salva o par no hashmap
            //o valor salvo não importa
            hashmap_set(mapParesEstacoes, par, 10, reg->codProxEstacao);
        }

        proxRRN++;
        free(reg);
    }

    atualizarStatus(file, '1', true);
    atualizarProxRRN(file, proxRRN, true);
    atualizarNroEstacoes(file, hashmap_size(mapEstacoes), false); //false porque nroEstacoes é o campo seguinte de proxRRN
    atualizarNroParesEstacoes(file, hashmap_size(mapParesEstacoes), false); //false porque nroParesEstacoes é o campo seguinte de nroEstacoes

    hashmap_iterate(mapEstacoes, freeMapKeys, NULL);
    hashmap_free(mapEstacoes);
    hashmap_iterate(mapParesEstacoes, freeMapKeys, NULL);
    hashmap_free(mapParesEstacoes);

    free(linha);

    fclose(csv);
    fclose(file);
    return true;
}

bool criarIndiceArvoreB(FILE *fileDados, FILE *fileIndice) {
    char statusDados;
    bool ok = true;
    long offsetRegistro;

    if(!fileDados || !fileIndice) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    if(!escreverCabecalhoIndice(fileIndice, '0', -1, -1, 0, 0)) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    if(fseek(fileDados, TAM_CABECALHO, SEEK_SET) != 0) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    offsetRegistro = TAM_CABECALHO;

    offsetRegistro = TAM_CABECALHO;

    int nroNos = 0;
    while (1) {
        char removido;
        int lixo, chave, tamNomeEstacao, tamNomeLinha;
        int i;

        if (fread(&removido, sizeof(char), 1, fileDados) != 1) break;

        if (removido == '1') {
            if (fseek(fileDados, TAM_REG - 1, SEEK_CUR) != 0) {
                ok = false;
                break;
            }
            offsetRegistro += TAM_REG;
            continue;
        }

        if (fread(&lixo, sizeof(int), 1, fileDados) != 1 || fread(&chave, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }

        for (i = 0; i < 5; i++) {
            if (fread(&lixo, sizeof(int), 1, fileDados) != 1) {
                ok = false;
                break;
            }
        }
        if (!ok) break;

        if (fread(&tamNomeEstacao, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }
        if (tamNomeEstacao > 0 && fseek(fileDados, tamNomeEstacao, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        if (fread(&tamNomeLinha, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }
        if (tamNomeLinha > 0 && fseek(fileDados, tamNomeLinha, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        i = TAM_LIVRE_REG(tamNomeEstacao, tamNomeLinha);
        if (i > 0 && fseek(fileDados, i, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        if (insertIndice(fileIndice, chave, (int)offsetRegistro, &nroNos) == ERRO_DE_INSERCAO) {
            ok = false;
            break;
        }

        offsetRegistro += TAM_REG;
    }

    if (ok) {
        fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
        fwrite(&nroNos, sizeof(int), 1, fileIndice);
    }

    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    return true;
}
