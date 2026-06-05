#include "../../headers/indice/btree_no.h"
#include "../../headers/indice/btree_cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

/* (sabor implementação?)
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
*/

No *incializarNo(){
    No *no = (No*) malloc(sizeof(No));
    int *C = (int*) malloc(sizeof(int) * NRO_MAX_CHAVES);
    int *Pr = (int*) malloc(sizeof(int) * NRO_MAX_CHAVES);
    int *P = (int*) malloc(sizeof(int) * NRO_MAX_CHAVES);

    no->proximo = -1;
    no->tipoNo = NO_NAO_INICIALIZADO;
    no->nroChaves = 0;
    memset(C, -1, sizeof(int) * NRO_MAX_CHAVES);
    memset(Pr, -1, sizeof(int) * NRO_MAX_CHAVES);
    memset(P, -1, sizeof(int) * (NRO_MAX_CHAVES+1));
    no->removido = '0';
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
    fseek(fileIndice, 5, SEEK_SET);

    int topo;
    fread(&topo, sizeof(int), 1, fileIndice);

    int proxRRN;
    fread(&proxRRN, sizeof(int), 1, fileIndice);

    if(topo == -1){
        *novoRRN = proxRRN;

        proxRRN++;
        fseek(fileIndice, 9, SEEK_SET);
        fwrite(&proxRRN, sizeof(int), 1, fileIndice);
    } else {
        //reaproveita um nó removido logicamente
        *novoRRN = topo;
        int inicioNoRemovido = TAM_BTREE_CABECALHO + topo * TAM_NO;

        //lê o novo topo
        int proximo;
        fseek(fileIndice, inicioNoRemovido+1, SEEK_SET); //+1 para pular a flag de removido
        fread(&proximo, sizeof(int), 1, fileIndice);
        
        //atualiza o topo da pilha de removidos no cabeçalho
        fseek(fileIndice, 5, SEEK_SET);
        fwrite(&proximo, sizeof(int), 1, fileIndice);
    }
    return incializarNo();
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


bool insereOrdenado(No *no, int chave, int ponteiroDados, int filhoDir){
    if(no->nroChaves >= NRO_MAX_CHAVES) return false;

    int i = no->nroChaves - 1;
    //shifta todos os elementos maiores do que a chave para a direita
    while (i >= 0 && no->C[i] > chave) {
        no->C[i+1] = no->C[i];
        no->P[i+1] = no->P[i];
        no->Pr[i+1] = no->Pr[i];
        i--;
    }

    //insere na posição correta
    int posicaoInsercao = i + 1;
    no->C[posicaoInsercao] = chave;
    no->Pr[posicaoInsercao] = ponteiroDados;
    no->P[posicaoInsercao] = filhoDir;

    no->nroChaves++;
    return true;
}

int comparaInt(const void *a, const void *b) {
    int n1 = *(const int *)a;
    int n2 = *(const int *)b;
    return (n1 > n2) - (n1 < n2);
}

/* (ideia de implementação) */
int distribuirOrdenado(FILE *fileIndice, No *no, No *novoNo, int chaveNova, int filhoDirChaveNova){
    return 0;
}