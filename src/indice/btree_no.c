#include "../../headers/indice/btree_no.h"
#include "../../headers/indice/btree_cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static void limparNo(No *no) {
    no->removido = '0';
    no->proximo = -1;
    no->tipoNo = NO_NAO_INICIALIZADO;
    no->nroChaves = 0;

    for (int i = 0; i < NRO_MAX_CHAVES; i++) {
        no->C[i] = -1;
        no->Pr[i] = -1;
        no->P[i] = -1;
    }
    no->P[NRO_MAX_CHAVES] = -1;
}

No *inicializarNo(){
    No *no = (No*) malloc(sizeof(No));
    int *C = (int*) malloc(sizeof(int) * NRO_MAX_CHAVES);
    int *Pr = (int*) malloc(sizeof(int) * NRO_MAX_CHAVES);
    int *P = (int*) malloc(sizeof(int) * (NRO_MAX_CHAVES+1));
    
    limparNo(no);
    return no;
}

void lerNo(FILE *fileIndice, No *no){
    fread(&no->removido, sizeof(char), 1, fileIndice);
    fread(&no->proximo, sizeof(int), 1, fileIndice);
    fread(&no->tipoNo, sizeof(int), 1, fileIndice);
    fread(&no->nroChaves, sizeof(int), 1, fileIndice);
    fread(&no->C[0], sizeof(int), 1, fileIndice);
    fread(&no->Pr[0], sizeof(int), 1, fileIndice);
    fread(&no->C[1], sizeof(int), 1, fileIndice);
    fread(&no->Pr[1], sizeof(int), 1, fileIndice);
    fread(&no->C[2], sizeof(int), 1, fileIndice);
    fread(&no->Pr[2], sizeof(int), 1, fileIndice);
    fread(&no->P, sizeof(int) * (NRO_MAX_CHAVES+1), 1, fileIndice);
}

No *criarNo(FILE *fileIndice, int *novoRRN){
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);

    int topo;
    fread(&topo, sizeof(int), 1, fileIndice);

    int proxRRN;
    fread(&proxRRN, sizeof(int), 1, fileIndice);

    if(topo == -1){
        *novoRRN = proxRRN;
        proxRRN++;
        fseek(fileIndice, BTREE_OFF_PROXRRN, SEEK_SET);
        fwrite(&proxRRN, sizeof(int), 1, fileIndice);
    } else {
        //reaproveita um nó removido logicamente
        *novoRRN = topo;
        int inicioNoRemovido = BTREE_NO_INICIO(topo);

        //lê o novo topo
        int novoTopo;
        fseek(fileIndice, inicioNoRemovido+1, SEEK_SET); //+1 para pular a flag de removido
        fread(&novoTopo, sizeof(int), 1, fileIndice);
        
        //atualiza o topo da pilha de removidos no cabeçalho
        fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
        fwrite(&novoTopo, sizeof(int), 1, fileIndice);
    }
    return inicializarNo();
}

bool escreverNo(FILE *fileIndice, No *no){
    fwrite(&no->removido, sizeof(char), 1, fileIndice);
    fwrite(&no->proximo, sizeof(int), 1, fileIndice);
    fwrite(&no->tipoNo, sizeof(int), 1, fileIndice);
    fwrite(&no->nroChaves, sizeof(int), 1, fileIndice);
    fwrite(&no->C[0], sizeof(int), 1, fileIndice);
    fwrite(&no->Pr[0], sizeof(int), 1, fileIndice);
    fwrite(&no->C[1], sizeof(int), 1, fileIndice);
    fwrite(&no->Pr[1], sizeof(int), 1, fileIndice);
    fwrite(&no->C[2], sizeof(int), 1, fileIndice);
    fwrite(&no->Pr[2], sizeof(int), 1, fileIndice);
    fwrite(&no->P, sizeof(int) * (NRO_MAX_CHAVES+1), 1, fileIndice);
    return true;
}

bool encontrarChave(No *no, int chave, int *subArvore, int *ponteiroDados){
    int C1 = no->C[0] != -1 ? no->C[0] : INT_MAX;
    int C2 = no->C[1] != -1 ? no->C[1] : INT_MAX;
    int C3 = no->C[2] != -1 ? no->C[2] : INT_MAX;

    if(chave == C2){
        *ponteiroDados = no->Pr[1];
        return true;
    }
    else if(chave < C2){
        if(chave == C1){
            *ponteiroDados = no->Pr[0];
            return true;
        } else if(chave < C1) {
            *subArvore = no->P[0];
        }
        else {
            *subArvore = no->P[1];
        }
    }
    else {
        if(chave == C3){
            *ponteiroDados = no->Pr[2];
        return true;
    }
        else if(chave < C3){
            *subArvore = no->P[2];
        }
        else {
            *subArvore = no->P[3];
        }
    }
    return false;
}


bool insereOrdenado(No *no, ElementoIndice elem){
    if(no->nroChaves >= NRO_MAX_CHAVES) return false;

    int i = no->nroChaves - 1;
    //shifta todos os elementos maiores do que a chave para a direita
    while (i >= 0 && no->C[i] > elem.chave) {
        no->C[i+1] = no->C[i];
        no->Pr[i+1] = no->Pr[i];
        no->P[i+2] = no->P[i+1]; //o filho a direita acompanha a chave
        i--;
    }

    //insere na posição correta
    int posicaoInsercao = i + 1;
    no->C[posicaoInsercao] = elem.chave;
    no->Pr[posicaoInsercao] = elem.ptrDados;
    no->P[posicaoInsercao+1] = elem.filhoDir;

    no->nroChaves++;
    return true;
}

void insertionSort(ElementoIndice arr[], int tam) {
    for (int i = 1; i < tam; i++) {
        ElementoIndice chaveAtual = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].chave > chaveAtual.chave) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = chaveAtual;
    }
}

void distribuirUniforme(FILE *fileIndice, No *no, No *novoNo, int rrnNovoNo, ElementoIndice overflowElem, ElementoIndice *promoPraCima){
    ElementoIndice elems[4];

    elems[0] = (ElementoIndice){no->C[0], no->Pr[0], no->P[1]};
    elems[1] = (ElementoIndice){no->C[1], no->Pr[1], no->P[2]};
    elems[2] = (ElementoIndice){no->C[2], no->Pr[2], no->P[3]};
    elems[3] = overflowElem;

    int ponteiroMaisEsquerda = no->P[0];

    insertionSort(elems, 4);

    no->C[0] = elems[0].chave;
    no->Pr[0] = elems[0].ptrDados;
    no->P[1] = elems[0].filhoDir;

    no->C[1] = elems[1].chave;
    no->Pr[1] = elems[1].ptrDados;
    no->P[2] = elems[1].filhoDir;

    no->C[2] = -1;
    no->Pr[2] = -1;
    no->P[3] = -1;
    no->P[0] = ponteiroMaisEsquerda;
    no->nroChaves = 2;

    promoPraCima->chave = elems[2].chave;
    promoPraCima->ptrDados = elems[2].ptrDados;
    promoPraCima->filhoDir = rrnNovoNo;

    novoNo->C[0] = elems[3].chave;
    novoNo->Pr[0]= elems[3].ptrDados;
    novoNo->P[0] = elems[2].filhoDir;
    novoNo->P[1] = elems[3].filhoDir;

    novoNo->C[1] = -1; novoNo->C[2]  = -1;
    novoNo->Pr[1] = -1; novoNo->Pr[2] = -1;
    novoNo->P[2] = -1; novoNo->P[3]  = -1;
    
    novoNo->nroChaves = 1;
    novoNo->tipoNo = no->tipoNo;
}

void apagarNo(FILE *fileIndice, int rrnNoParaApagar) {
    int topoAtual;
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
    fread(&topoAtual, sizeof(int), 1, fileIndice);

    // Altera apenas o removido e o encadeamento (mantém os bytes antigos intactos)
    fseek(fileIndice, BTREE_NO_INICIO(rrnNoParaApagar), SEEK_SET);
    char removido = '1';
    fwrite(&removido, sizeof(char), 1, fileIndice);
    fwrite(&topoAtual, sizeof(int), 1, fileIndice);

    // Atualiza o topo
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
    fwrite(&rrnNoParaApagar, sizeof(int), 1, fileIndice);

    // Decrementa estritamente o nroNos, como pede a especificação do trabalho 2
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    int nroNos;
    fread(&nroNos, sizeof(int), 1, fileIndice);
    nroNos--;
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    fwrite(&nroNos, sizeof(int), 1, fileIndice);
}