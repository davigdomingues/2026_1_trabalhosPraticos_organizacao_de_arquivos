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

void apagarNo(FILE *fileIndice, int rrnNoParaApagar, int *nroNos) {
    int topoAtual;
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
    fread(&topoAtual, sizeof(int), 1, fileIndice);

    // Altera apenas o removido e o encadeamento (mantém os bytes antigos intactos)
    fseek(fileIndice, BTREE_NO_INICIO(rrnNoParaApagar), SEEK_SET);
    char removido = '1';
    fwrite(&removido, sizeof(char), 1, fileIndice);
    fwrite(&topoAtual, sizeof(int), 1, fileIndice);

    // Atualiza o topo da pilha de nós excluídos
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
    fwrite(&rrnNoParaApagar, sizeof(int), 1, fileIndice);

    if (nroNos != NULL) (*nroNos)--; // Decrementa o contador de nós da Árvore-B, apenas em memória, sem alterar o arquivo
}