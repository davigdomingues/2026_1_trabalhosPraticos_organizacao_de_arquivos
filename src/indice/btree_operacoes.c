#include "../../headers/indice/btree_operacoes.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/indice/btree_no.h"
#include "../../headers/dados/operacoes.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros){
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

bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados){
    if(rrnNoAtual == -1) return false;

    int inicioNo = TAM_BTREE_CABECALHO + rrnNoAtual * TAM_NO;
    fseek(fileIndice, inicioNo, SEEK_SET);

    No *no = incializarNo();
    lerNo(fileIndice, no);

    int subArvore;
    bool encontrou = encontrarChave(no, chave, &subArvore, ponteiroDados);
    if(!encontrou){
        return buscaRecursiva(fileIndice, chave, subArvore, rrnNoRes, ponteiroDados);
    }

    *rrnNoRes = rrnNoAtual;
    return true;
}

No *split(FILE *fileIndice, No *no, int chaveNova, int ponteiroDadosChaveNova, int filhoDirChaveNova, int *chaveASerPromovida, int *filhoDirChaveASerPromovida){
    int rrnNovoNo;
    No *novoNo = criarNo(fileIndice, &rrnNovoNo);
    int chaveMeio = distribuirOrdenado(fileIndice, no, novoNo, chaveNova, filhoDirChaveNova);
    *chaveASerPromovida = chaveMeio;
    *filhoDirChaveASerPromovida = rrnNovoNo;

    return novoNo;
}

void criarNovaRaiz(FILE *fileIndice, int rrnRaizAtual, int chavePromocao, int filhoDirPromocao){
    int rrnNovaRaiz;
    No *novaRaiz = criarNo(fileIndice, &rrnNovaRaiz);
        
    novaRaiz->nroChaves = 1;
    novaRaiz->C[0] = chavePromocao;
    novaRaiz->P[0] = rrnRaizAtual;          //filho a esquerda é a raiz antiga
    novaRaiz->P[1] = filhoDirPromocao; //filho a direita é o nó criado no split
    novaRaiz->tipoNo = NO_RAIZ;

    //salva a nova raiz no arquivo
    int inicioNovaRaiz = TAM_BTREE_CABECALHO + rrnNovaRaiz * TAM_NO;
    fseek(fileIndice, inicioNovaRaiz, SEEK_SET);
    escreverNo(fileIndice, novaRaiz);

    //atualiza o cabeçalho do arquivo para apontar para o RRN da nova raiz
    fseek(fileIndice, 1, SEEK_SET); 
    fwrite(&rrnNovaRaiz, sizeof(int), 1, fileIndice);
        
    free(novaRaiz);
}

int insertIndice(FILE *fileIndice, int chave, int ponteiroDados){
    int rrnRaiz;
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    int chavePromocao;
    int filhoDirPromocao;
    int res = insertIndiceRec(fileIndice, chave, ponteiroDados, rrnRaiz, &chavePromocao, &filhoDirPromocao);
    if(res == PROMOCAO) criarNovaRaiz(fileIndice, rrnRaiz, chavePromocao, filhoDirPromocao);

    //atualiza o número de nós da árvore
    fseek(fileIndice, 13, SEEK_SET);
    int nroNos;
    fread(&nroNos, sizeof(int), 1, fileIndice);
    nroNos++;
    fseek(fileIndice, 13, SEEK_SET);
    fwrite(&nroNos, sizeof(int), 1, fileIndice);

    //atualiza o status de consistência
    fseek(fileIndice, 0, SEEK_SET);
    char consistente = '1';
    fwrite(&consistente, sizeof(char), 1, fileIndice);
    return 1;
}

int insertIndiceRec(FILE *fileIndice, int chave, int ponteiroDados, int rrnNoAtual, int *chaveASerPromovida, int *filhoDirChaveASerPromovida){
    if(rrnNoAtual == -1){
        *chaveASerPromovida = chave;
        *filhoDirChaveASerPromovida = -1;
        return PROMOCAO;
    }

    int inicioNo = TAM_BTREE_CABECALHO + rrnNoAtual * TAM_NO;
    fseek(fileIndice, inicioNo, SEEK_SET);

    No *no = incializarNo();
    lerNo(fileIndice, no);

    int subArvore;
    int dummy; //não vai ser utilizado
    bool encontrou = encontrarChave(no, chave, &subArvore, &dummy);
    if(encontrou) return ERRO_DE_INSERCAO;

    int chavePromovida;
    int filhoDirPromovido;
    int res = insertIndiceRec(fileIndice, chave, ponteiroDados, subArvore, &chavePromovida, &filhoDirPromovido);

    if(res == SEM_PROMOCAO || res == ERRO_DE_INSERCAO) return res;
    else if (no->nroChaves < NRO_MAX_CHAVES){
        inserirOrdenado(no, chavePromovida, ponteiroDados, filhoDirPromovido);
        no->tipoNo = NO_FOLHA;

        fseek(fileIndice, inicioNo, SEEK_SET);
        escreverNo(fileIndice, no);
        return SEM_PROMOCAO;
    } else {
        No *novoNo = split(fileIndice, no, chavePromovida, ponteiroDados, filhoDirPromovido, chaveASerPromovida, filhoDirChaveASerPromovida);

        fseek(fileIndice, inicioNo, SEEK_SET);
        escreverNo(fileIndice, no);
        no->tipoNo = NO_INTERMEDIARIO;

        int inicioNovoNo = TAM_BTREE_CABECALHO + *filhoDirChaveASerPromovida * TAM_NO;
        fseek(fileIndice, inicioNovoNo, SEEK_SET);
        escreverNo(fileIndice, novoNo);
        return PROMOCAO;
    }

    return false;
}