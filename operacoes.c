#include "operacoes.h"
#include "registro.h"
#include "cabecalho.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fornecidas.h"
#include "c-hashmap/map.h" //usando uma biblioteca, por enquanto. créditos da biblioteca para Mashpoe.

int freeMapKeys(const void* key, size_t ksize, uintptr_t value, void* usr){
    free((void*) key);
    return 0;
}

static int encontrarIndexCampo(CampoValor *pares, int mPares, const char *campo) {
    for (int i = 0; i < mPares; i++) {
        if (strcmp(pares[i].campo, campo) == 0) return i;
    }
    return -1;
}

void create(char *arquivoEntrada, char *arquivoSaida){
    FILE *file = fopen(arquivoSaida, "wb");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    inicializarCabecalho(file);

    int proxRRN = 0;
    //cria um hashmap para depois obter, eficientemente, o nroEstacoes únicas
    hashmap *mapEstacoes = hashmap_create();
    //cria um hashmap para depois obter, eficientemente, o nroParesEstacoes únicas
    hashmap *mapParesEstacoes = hashmap_create();

    FILE *csv = fopen(arquivoEntrada, "r");
    if(!csv){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

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

void selectAll(char *arquivoEntrada){
    FILE *file = fopen(arquivoEntrada, "rb");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    fseek(file, TAM_CABECALHO, SEEK_SET); 

    Registro *reg = (Registro*) malloc(sizeof(Registro));
    int regLidos = 0;
    char removido;
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        fseek(file, 4, SEEK_CUR); //pula os 4 bytes de proxRRN


        //lê os campos do registro e armazena na struct
        fread(&reg->codEstacao, sizeof(int), 1, file);
        fread(&reg->codLinha, sizeof(int), 1, file);
        fread(&reg->codProxEstacao, sizeof(int), 1, file);
        fread(&reg->distProxEstacao, sizeof(int), 1, file);
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
        regLidos++;

        printReg(reg);
        int tamRestante = TAM_REG - 9 * sizeof(int) - sizeof(char) - reg->tamNomeEstacao - reg->tamNomeLinha;
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        free(reg->nomeEstacao);
        free(reg->nomeLinha);
    }

    if(regLidos == 0) printf("Registro inexistente.\n");
    free(reg);
}

bool verificarMatchInt(int index, char *valorQuery, int valorReg) {
    //a query inclui um valor a ser buscado para esse campo?
    if (index > -1) {
        //o valor a ser buscado para esse campo é nulo e o valor do registro atual também é?
        //ou o valor a ser buscado para esse campo não é nulo e é igual ao valor do registro atual?
        if ((*valorQuery == '\0' && valorReg == -1) || atoi(valorQuery) == valorReg) {
            //se sim, houve um match
            return true;
        }
    }
    //se não, não houve match
    return false;
}

bool verificarMatchStr(int index, char *valorQuery, char *valorReg) {
    //a query inclui um valor a ser buscado para esse campo
    //e os valores são iguais?
    if (index > -1 && strcmp(valorQuery, valorReg) == 0) { 
        //OBS: a comparação também funciona pros casos nulos
        //porque, simplesmente, a comparação é feita com strings vazias
        return true;
    }
    //se não, não houve match
    return false;
}

void selectWhere(char *arquivoEntrada, CampoValor *pares, int mPares){
    FILE *file = fopen(arquivoEntrada, "rb");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    //indica em qual posição está o par campo-valor em questão
    //e, consequentemente, se a busca inclui um valor a ser buscado
    //para esse campo
    int indexCodEstacao = -1;
    int indexNomeEstacao = -1;
    int indexCodLinha = -1;
    int indexNomeLinha = -1;
    int indexCodProxEstacao = -1;
    int indexDistProxEstacao = -1;
    int indexCodLinhaIntegra = -1;
    int indexCodEstIntegra = -1;
    for (int i = 0; i < mPares; i++) {
        if(strcmp(pares[i].campo, "codEstacao") == 0){
            indexCodEstacao = i;
        } if(strcmp(pares[i].campo, "nomeEstacao") == 0){
            indexNomeEstacao = i;
        } if(strcmp(pares[i].campo, "codLinha") == 0){
            indexCodLinha = i;
        } if(strcmp(pares[i].campo, "nomeLinha") == 0){
            indexNomeLinha = i;
        } if(strcmp(pares[i].campo, "codProxEstacao") == 0){
            indexCodProxEstacao = i;
        } if(strcmp(pares[i].campo, "distProxEstacao") == 0){
            indexDistProxEstacao = i;
        } if(strcmp(pares[i].campo, "codLinhaIntegra") == 0){
            indexCodLinhaIntegra = i;
        } if(strcmp(pares[i].campo, "codEstIntegra") == 0){
            indexCodEstIntegra = i;
        }
    }

    int numMatches = 0;
    Registro *reg = (Registro*) malloc(sizeof(Registro));
    char removido;
    fseek(file, TAM_CABECALHO, SEEK_SET); //pula o cabeçalho
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        fseek(file, 4, SEEK_CUR); //pula os 4 bytes de proxRRN

        fread(&reg->codEstacao, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodEstacao, pares[indexCodEstacao].valor, reg->codEstacao)) numMatches++;

        fread(&reg->codLinha, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodLinha, pares[indexCodLinha].valor, reg->codLinha)) numMatches++;

        fread(&reg->codProxEstacao, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodProxEstacao, pares[indexCodProxEstacao].valor, reg->codProxEstacao)) numMatches++;

        fread(&reg->distProxEstacao, sizeof(int), 1, file);
        if (verificarMatchInt(indexDistProxEstacao, pares[indexDistProxEstacao].valor, reg->distProxEstacao)) numMatches++;

        fread(&reg->codLinhaIntegra, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodLinhaIntegra, pares[indexCodLinhaIntegra].valor, reg->codLinhaIntegra)) numMatches++;

        fread(&reg->codEstIntegra, sizeof(int), 1, file);
        if (verificarMatchInt(indexCodEstIntegra, pares[indexCodEstIntegra].valor, reg->codEstIntegra)) numMatches++;

        fread(&reg->tamNomeEstacao, sizeof(int), 1, file);
        //lê o nomeEstacao, se não for um campo NULO
        if(reg->tamNomeEstacao != 0){
            char *nomeEstacao = (char*) malloc( sizeof(char) * reg->tamNomeEstacao);
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        } else {
            //se for NULO, só indica que é
            reg->nomeEstacao = "";
        }
        if(verificarMatchStr(indexNomeEstacao, pares[indexNomeEstacao].valor, reg->nomeEstacao)) numMatches++;

        fread(&reg->tamNomeLinha, sizeof(int), 1, file);
        //lê o nomeLinha, se não for um campo NULO
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc(sizeof(char) * reg->tamNomeLinha);
            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, file);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;
        } else {
            //se for NULO, só indica que é
            reg->nomeLinha = "";
        }
        if(verificarMatchStr(indexNomeLinha, pares[indexNomeLinha].valor, reg->nomeLinha)) numMatches++;

        if(numMatches == mPares) printReg(reg);
        int tamRestante = TAM_REG - 9 * sizeof(int) - sizeof(char) - reg->tamNomeEstacao - reg->tamNomeLinha;
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        numMatches = 0;
        free(reg->nomeEstacao);
        free(reg->nomeLinha);
    }
    fclose(file);
    return;
}

bool deleteWhere(char *arquivoEntrada, CampoValor *pares, int mPares){
    FILE *file = fopen(arquivoEntrada, "r+b");
    if(!file){
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    char status;
    int topo;

    // lê o status e o topo da lista de removidos do cabeçalho
    if (fread(&status, sizeof(char), 1, file) != 1 || status != '1' || fread(&topo, sizeof(int), 1, file) != 1) {
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return false;
    }

    fseek(file, 0, SEEK_SET);
    atualizarStatus(file, '0', false);

    // indica em qual posição está o par campo-valor
    int indexCodEstacao = encontrarIndexCampo(pares, mPares, "codEstacao");
    int indexNomeEstacao = encontrarIndexCampo(pares, mPares, "nomeEstacao");
    int indexCodLinha = encontrarIndexCampo(pares, mPares, "codLinha");
    int indexNomeLinha = encontrarIndexCampo(pares, mPares, "nomeLinha");
    int indexCodProxEstacao = encontrarIndexCampo(pares, mPares, "codProxEstacao");
    int indexDistProxEstacao = encontrarIndexCampo(pares, mPares, "distProxEstacao");
    int indexCodLinhaIntegra = encontrarIndexCampo(pares, mPares, "codLinhaIntegra");
    int indexCodEstIntegra = encontrarIndexCampo(pares, mPares, "codEstIntegra");

    fseek(file, TAM_CABECALHO, SEEK_SET);
    bool ok = true;
    
    // varre o arquivo lendo os registros um por um, verificando se cada um deles satisfaz as condições de remoção e, caso sim, removendo-o (ou seja, marcando como removido e atualizando a lista de removidos)
    while (ok) {
        char removido;
        size_t lido = fread(&removido, sizeof(char), 1, file);
        if (lido != 1) break; // Fim do arquivo (EOF)

        long inicioRegistro = ftell(file) - 1;

        if (removido == '1') {
            fseek(file, TAM_REG - 1, SEEK_CUR);
            continue;
        }

        int proximo;
        Registro reg;
        if (fread(&proximo, sizeof(int), 1, file) != 1) { ok = false; break; }
        reg.proximo = proximo;

        // lê os campos do registro e armazena na struct
        if (fread(&reg.codEstacao, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codLinha, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codProxEstacao, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.distProxEstacao, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codLinhaIntegra, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (fread(&reg.codEstIntegra, sizeof(int), 1, file) != 1) { ok = false; break; }

        // lê o nomeEstacao, se não for um campo NULO
        char *nomeEstacao = "";
        if (fread(&reg.tamNomeEstacao, sizeof(int), 1, file) != 1) { ok = false; break; }
        if (reg.tamNomeEstacao > 0) {
            nomeEstacao = (char*) malloc((size_t)reg.tamNomeEstacao + 1);
            if (!nomeEstacao) { ok = false; break; }
            if (fread(nomeEstacao, sizeof(char), reg.tamNomeEstacao, file) != (size_t)reg.tamNomeEstacao) {
                free(nomeEstacao);
                ok = false;
                break;
            }
            nomeEstacao[reg.tamNomeEstacao] = '\0';
        }

        // lê o nomeLinha, seguindo mesma lógica de nomeEstacao
        char *nomeLinha = "";
        if (fread(&reg.tamNomeLinha, sizeof(int), 1, file) != 1) {
            if (reg.tamNomeEstacao > 0) free(nomeEstacao);
            ok = false;
            break;
        }
        if (reg.tamNomeLinha > 0) {
            nomeLinha = (char*) malloc((size_t)reg.tamNomeLinha + 1);
            if (!nomeLinha) {
                if (reg.tamNomeEstacao > 0) free(nomeEstacao);
                ok = false;
                break;
            }
            if (fread(nomeLinha, sizeof(char), reg.tamNomeLinha, file) != (size_t)reg.tamNomeLinha) {
                if (reg.tamNomeEstacao > 0) free(nomeEstacao);
                free(nomeLinha);
                ok = false;
                break;
            }
            nomeLinha[reg.tamNomeLinha] = '\0';
        }

        int numMatches = 0;
        
        // verifica se o registro atual satisfaz as condições de remoção e se os campos para os quais a busca inclui um valor a ser buscado satisfazem a condição de match.
        if (indexCodEstacao > -1 && verificarMatchInt(indexCodEstacao, pares[indexCodEstacao].valor, reg.codEstacao)) numMatches++;
        if (indexCodLinha > -1 && verificarMatchInt(indexCodLinha, pares[indexCodLinha].valor, reg.codLinha)) numMatches++;
        if (indexCodProxEstacao > -1 && verificarMatchInt(indexCodProxEstacao, pares[indexCodProxEstacao].valor, reg.codProxEstacao)) numMatches++;
        if (indexDistProxEstacao > -1 && verificarMatchInt(indexDistProxEstacao, pares[indexDistProxEstacao].valor, reg.distProxEstacao)) numMatches++;
        if (indexCodLinhaIntegra > -1 && verificarMatchInt(indexCodLinhaIntegra, pares[indexCodLinhaIntegra].valor, reg.codLinhaIntegra)) numMatches++;
        if (indexCodEstIntegra > -1 && verificarMatchInt(indexCodEstIntegra, pares[indexCodEstIntegra].valor, reg.codEstIntegra)) numMatches++;
        if (indexNomeEstacao > -1 && verificarMatchStr(indexNomeEstacao, pares[indexNomeEstacao].valor, nomeEstacao)) numMatches++;
        if (indexNomeLinha > -1 && verificarMatchStr(indexNomeLinha, pares[indexNomeLinha].valor, nomeLinha)) numMatches++;

        int tamRestante = TAM_REG - 9 * (int)sizeof(int) - (int)sizeof(char) - reg.tamNomeEstacao - reg.tamNomeLinha;
        if (tamRestante < 0 || (tamRestante != 0 && fseek(file, tamRestante, SEEK_CUR) != 0)) {
            if (reg.tamNomeEstacao > 0) free(nomeEstacao);
            if (reg.tamNomeLinha > 0) free(nomeLinha);
            ok = false;
            break;
        }

        if (numMatches == mPares) {
            int rrn = (int)((inicioRegistro - TAM_CABECALHO) / TAM_REG);

            if (fseek(file, inicioRegistro, SEEK_SET) != 0) {
                if (reg.tamNomeEstacao > 0) free(nomeEstacao);
                if (reg.tamNomeLinha > 0) free(nomeLinha);
                ok = false;
                break;
            }

            char removidoFlag = '1';
            if (fwrite(&removidoFlag, sizeof(char), 1, file) != 1 || fwrite(&topo, sizeof(int), 1, file) != 1) {
                if (reg.tamNomeEstacao > 0) free(nomeEstacao);
                if (reg.tamNomeLinha > 0) free(nomeLinha);
                ok = false;
                break;
            }

            topo = rrn;
            // atualização do topo da lista de removidos no cabeçalho
            if (fseek(file, 1, SEEK_SET) != 0 || fwrite(&topo, sizeof(int), 1, file) != 1) {
                if (reg.tamNomeEstacao > 0) free(nomeEstacao);
                if (reg.tamNomeLinha > 0) free(nomeLinha);
                ok = false;
                break;
            }

            // volta para após o registro e continua a varredura normalmente, para o caso de haver mais de um registro a ser removido
            if (fseek(file, inicioRegistro + TAM_REG, SEEK_SET) != 0) {
                if (reg.tamNomeEstacao > 0) free(nomeEstacao);
                if (reg.tamNomeLinha > 0) free(nomeLinha);
                ok = false;
                break;
            }
        }

        if (reg.tamNomeEstacao > 0) free(nomeEstacao);
        if (reg.tamNomeLinha > 0) free(nomeLinha);
    }

    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return false;
    }

    fseek(file, 0, SEEK_SET);
    atualizarStatus(file, '1', false);
    fclose(file);
    return true;
}