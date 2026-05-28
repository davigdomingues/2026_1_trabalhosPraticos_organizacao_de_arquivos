#include "../../headers/indice/btree_operacoes.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/indice/btree_no.h"
#include "../../headers/dados/operacoes.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int selectWhereIndexado(FILE *fileDados, CampoValor *pares[8], int numFiltros){
    FILE *fileIndice = fopen("estacoesRemBTree.bin", "rb");
    if (!fileIndice) {
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    int chave = atoi(pares[CAMPO_COD_ESTACAO]->valor);
    int rrnRaiz;
    fseek(fileIndice, 1, SEEK_SET);
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    int rrnRes;
    int ponteiroRes;
    bool encontrou = buscaRecursiva(fileIndice, chave, rrnRaiz, &rrnRes, &ponteiroRes);

    if(!encontrou) return -1;
    else {
        fseek(fileDados, ponteiroRes, SEEK_SET);

        char removido;
        fread(&removido, sizeof(char), 1, fileDados);
        if(removido == '1'){
            printf("Registro inexistente.\n");
            return -1;
        }

        pares[CAMPO_COD_ESTACAO] = NULL; //retira codEstacao dos criterios de busca
        Registro *reg = (Registro*) malloc(sizeof(Registro));
        int numMatches = confereCriteriosBusca(fileDados, reg, pares);

        //numFiltros-1 porque o codEstacao foi retirado dos critérios de busca
        if(numMatches != numFiltros-1){
            printf("Registro inexistente.\n");
            return -1;
        }

        printReg(reg);
        return ponteiroRes;
    }
}

bool buscaRecursiva(FILE *fileIndice, int chave, int rrn, int *rrnRes, int *ponteiroRes){
    if(rrn == -1) return false;

    int inicioNo = TAM_BTREE_CABECALHO + rrn * TAM_NO;
    fseek(fileIndice, inicioNo, SEEK_SET);

    No no = incializarNo();
    fread(&no.removido, sizeof(char), 1, fileIndice);
    fread(&no.proximo, sizeof(int), 1, fileIndice);
    fread(&no.tipoNo, sizeof(int), 1, fileIndice);
    fread(&no.nroChaves, sizeof(int), 1, fileIndice);
    fread(&no.C1, sizeof(int), 1, fileIndice);
    fread(&no.Pr1, sizeof(int), 1, fileIndice);
    fread(&no.C2, sizeof(int), 1, fileIndice);
    fread(&no.Pr2, sizeof(int), 1, fileIndice);
    fread(&no.C3, sizeof(int), 1, fileIndice);
    fread(&no.Pr3, sizeof(int), 1, fileIndice);
    fread(&no.P1, sizeof(int), 1, fileIndice);
    fread(&no.P2, sizeof(int), 1, fileIndice);
    fread(&no.P3, sizeof(int), 1, fileIndice);
    fread(&no.P4, sizeof(int), 1, fileIndice);

    int C1 = no.C1 != -1 ? no.C1 : INT_MAX;
    int C2 = no.C2 != -1 ? no.C2 : INT_MAX;
    int C3 = no.C3 != -1 ? no.C3 : INT_MAX;

    if(chave == C2){
        *rrnRes = rrn;
        *ponteiroRes = no.Pr2;
        return true;
    }
    else if(chave < C2){
        if(chave == no.C1){
            *rrnRes = rrn;
            *ponteiroRes = no.Pr1;
            return true;
        } else if(chave < C1) {
            return buscaRecursiva(fileIndice, chave, no.P1, rrnRes, ponteiroRes);
        }
        else {
            return buscaRecursiva(fileIndice, chave, no.P2, rrnRes, ponteiroRes);
        }
    }
    else {
        if(chave == C3){
            *rrnRes = rrn;
            *ponteiroRes = no.Pr3;
            return true;
        }
        else if(chave < C3){
            return buscaRecursiva(fileIndice, chave, no.P3, rrnRes, ponteiroRes);
        }
        else {
            return buscaRecursiva(fileIndice, chave, no.P4, rrnRes, ponteiroRes);
        }
    }
    return true;
}