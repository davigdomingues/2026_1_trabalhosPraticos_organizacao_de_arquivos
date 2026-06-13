#include "../../headers/operacoes/buscas.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/utils.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/indice/btree_no.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void selectAll(FILE *file){
    fseek(file, TAM_CABECALHO, SEEK_SET); 

    Registro *reg = (Registro*) malloc(sizeof(Registro));
    if(!reg){
        fclose(file);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int regLidos = 0;
    char removido;
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        // garante estado limpo por iteração (evita usar ponteiros antigos quando o campo é nulo)
        reg->tamNomeEstacao = 0; reg->nomeEstacao = "";
        reg->tamNomeLinha = 0; reg->nomeLinha   = "";

        fseek(file, DADOS_OFF_PROXRRN - 1, SEEK_CUR); //pula os 4 bytes de proxRRN

        //lê os campos do registro e armazena na struct
        fread(&reg->codEstacao, sizeof(int), 1, file);
        fread(&reg->codLinha, sizeof(int), 1, file);
        fread(&reg->codProxEstacao, sizeof(int), 1, file);
        fread(&reg->distProxEstacao, sizeof(int), 1, file);
        fread(&reg->codLinhaIntegra, sizeof(int), 1, file);
        fread(&reg->codEstIntegra, sizeof(int), 1, file);

        fread(&reg->tamNomeEstacao, sizeof(int), 1, file);
        if(reg->tamNomeEstacao != 0){
            char *nomeEstacao = (char*) malloc(( sizeof(char) * reg->tamNomeEstacao ) + 1); // +1 para o caractere nulo
            if (!nomeEstacao) break;
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        }

        fread(&reg->tamNomeLinha, sizeof(int), 1, file);
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc((sizeof(char) * reg->tamNomeLinha ) + 1); // +1 para o caractere nulo
            if (!nomeLinha) { 
                if (reg->tamNomeEstacao > 0) 
                    free(reg->nomeEstacao); 
                break; 
            }

            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, file);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;
        }
        regLidos++;

        printReg(reg);
        int tamRestante = TAM_LIVRE_REG(reg->tamNomeEstacao, reg->tamNomeLinha);
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
        if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
    }

    if(regLidos == 0) printf("Registro inexistente.\n");
    free(reg);
    fclose(file);
}

int selectAllWhere(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares){
    int res = selectWhere(fileDados, fileIndice, pares, mPares, 0, false, true);
    return res;
}

int selectWhere(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares, int rrnInicial, bool apenasPrimeiroRes, bool seek){
    if(!fileDados) return -1;

    //se seek == true, vai para o rrn de início da busca
    //se rrnInicial == 0, precisa pular o cabeçalho
    //caso contrário, assume que o ponteiro do arquivo já está no lugar certo
    if(seek || rrnInicial == 0){
        long byteOffsetInicial = (long)TAM_CABECALHO + (long)rrnInicial * (long)TAM_REG;
        fseek(fileDados, byteOffsetInicial, SEEK_SET);
    }

    // indexa os pares por campo (posição fixa), posições sem filtro ficam como NULL
    CampoValor *porCampo[NUM_CAMPOS_REGISTRO];
    int numFiltros = popularParesPorCampo(pares, mPares, porCampo);

    if(porCampo[CAMPO_COD_ESTACAO] && fileIndice != NULL){
        return selectWhereIndexado(fileDados, fileIndice, porCampo, numFiltros, true);
    }

    Registro *reg = (Registro*) malloc(sizeof(Registro));
    if(!reg) return -1;

    int rrnAtual = rrnInicial;
    int numMatches = 0;

    bool encontrou = false;
    char removido;
    while(fread(&removido, sizeof(char), 1, fileDados)){
        if(removido == '1') {
            fseek(fileDados, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            rrnAtual++;
            continue;
        }

        numMatches = confereCriteriosBusca(fileDados, reg, porCampo);

        if(numMatches == numFiltros) {
            encontrou = true;
            if(apenasPrimeiroRes){
                if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
                if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
                free(reg);
                return rrnAtual;
            } else {
                printReg(reg);
            }
        }
        int tamRestante = TAM_LIVRE_REG(reg->tamNomeEstacao, reg->tamNomeLinha);
        if(tamRestante != 0) fseek(fileDados, tamRestante, SEEK_CUR); //pula os $

        if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
        if (reg->tamNomeLinha > 0) free(reg->nomeLinha);

        numMatches = 0;
        rrnAtual++;
    }

    //só imprime na tela se buscou por todos os resultados
    if(!apenasPrimeiroRes && !encontrou) printf("Registro inexistente.\n");
    if(!apenasPrimeiroRes) printf("\n");
    free(reg);

    if(!encontrou) return -1;
    else return 0;
}

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros, bool print){
    int chave = atoi(pares[CAMPO_COD_ESTACAO]->valor);
    int rrnRaiz;
    fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    int rrnRes;
    int ponteiroRes;
    bool encontrou = buscaRecursiva(fileIndice, chave, rrnRaiz, &rrnRes, &ponteiroRes);

    if(!encontrou){
        if(print){
            printf("Registro inexistente.\n");
            printf("\n");
        }
        return -1;
    }
    else {
        fseek(fileDados, ponteiroRes, SEEK_SET);

        char removido;
        fread(&removido, sizeof(char), 1, fileDados);
        if(removido == '1'){
            if(print){
                printf("Registro inexistente.\n");
                printf("\n");
            }
            return -1;
        }

        pares[CAMPO_COD_ESTACAO] = NULL; //retira codEstacao dos criterios de busca
        Registro *reg = (Registro*) malloc(sizeof(Registro));
        int numMatches = confereCriteriosBusca(fileDados, reg, pares);

        //numFiltros-1 porque o codEstacao foi retirado dos critérios de busca
        if(numMatches != numFiltros-1){
            if(print){
                printf("Registro inexistente.\n");
                printf("\n");
            }
            return -1;
        }

        if(print){
            printReg(reg);
            printf("\n");
        }
        return ponteiroRes;
    }
}

bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados){
    if(rrnNoAtual == -1) return false;

    int inicioNo = BTREE_NO_INICIO(rrnNoAtual);
    fseek(fileIndice, inicioNo, SEEK_SET);

    No *no = inicializarNo();
    lerNo(fileIndice, no);

    int subArvore;
    bool encontrou = encontrarChave(no, chave, &subArvore, ponteiroDados);
    if(!encontrou){
        bool res = buscaRecursiva(fileIndice, chave, subArvore, rrnNoRes, ponteiroDados);
        free(no);
        return res;
    }

    *rrnNoRes = rrnNoAtual;
    return true;
}
