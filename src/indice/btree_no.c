#include "../../headers/indice/btree_no.h"
#include "../../headers/indice/btree_cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

static void limparNo(No *no) {
    int i;

    no->removido = '0';
    no->proximo = -1;
    no->tipoNo = NO_NAO_INICIALIZADO;
    no->nroChaves = 0;

    for (i = 0; i < NRO_MAX_CHAVES; i++) {
        no->C[i] = -1;
        no->Pr[i] = -1;
    }

    for (i = 0; i < NRO_MAX_CHAVES + 1; i++) {
        no->P[i] = -1;
    }
}

No *incializarNo(){
    No *no = (No*) malloc(sizeof(No));
    if (!no) return NULL;

    limparNo(no);
    return no;
}

void lerNo(FILE *fileIndice, No *no){
    int i;

    // lê os campos fixos do nó
    fread(&no->removido, sizeof(char), 1, fileIndice);
    fread(&no->proximo, sizeof(int), 1, fileIndice);
    fread(&no->tipoNo, sizeof(int), 1, fileIndice);
    fread(&no->nroChaves, sizeof(int), 1, fileIndice);

    // lê os arrays de chaves, ponteiros de dados e ponteiros de filhos
    for (i = 0; i < NRO_MAX_CHAVES; i++) {
        fread(&no->C[i], sizeof(int), 1, fileIndice);
        fread(&no->Pr[i], sizeof(int), 1, fileIndice);
    }

    fread(no->P, sizeof(int), NRO_MAX_CHAVES + 1, fileIndice);
}

No *criarNo(FILE *fileIndice, int *novoRRN){
    int topo, proxRRN, nroNos;

    if (!fileIndice || !novoRRN) return NULL;

    // gerenciamento de espaço livre via lista encadeada no campo "proximo" dos nós removidos
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
    fread(&topo, sizeof(int), 1, fileIndice);
    fread(&proxRRN, sizeof(int), 1, fileIndice);

    // se não houver nós removidos, o novo nó é criado no final do arquivo usando proxRRN. Caso contrário, o nó do topo da lista de removidos é reutilizado.
    if (topo == -1) {
        *novoRRN = proxRRN;
        proxRRN++;

        fseek(fileIndice, BTREE_OFF_PROXRRN, SEEK_SET);
        fwrite(&proxRRN, sizeof(int), 1, fileIndice);
    } else {
        int proximoTopo = -1;
        long inicioNoRemovido = TAM_BTREE_CABECALHO + (long)topo * (long)TAM_NO;

        *novoRRN = topo;

        fseek(fileIndice, inicioNoRemovido + 1, SEEK_SET);
        fread(&proximoTopo, sizeof(int), 1, fileIndice);

        fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
        fwrite(&proximoTopo, sizeof(int), 1, fileIndice);
    }

    // incrementa o número de nós do arquivo de índice
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    fread(&nroNos, sizeof(int), 1, fileIndice);
    nroNos++;

    // atualiza o número de nós no cabeçalhodo arquivo de índice
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    fwrite(&nroNos, sizeof(int), 1, fileIndice);

    return incializarNo();
}

bool escreverNo(FILE *fileIndice, No *no){
    int i;

    if (!fileIndice || !no) return false;

    if (fwrite(&no->removido, sizeof(char), 1, fileIndice) != 1) return false;
    if (fwrite(&no->proximo, sizeof(int), 1, fileIndice) != 1) return false;
    if (fwrite(&no->tipoNo, sizeof(int), 1, fileIndice) != 1) return false;
    if (fwrite(&no->nroChaves, sizeof(int), 1, fileIndice) != 1) return false;

    for (i = 0; i < NRO_MAX_CHAVES; i++) {
        if (fwrite(&no->C[i], sizeof(int), 1, fileIndice) != 1) return false;
        if (fwrite(&no->Pr[i], sizeof(int), 1, fileIndice) != 1) return false;
    }

    if (fwrite(no->P, sizeof(int), NRO_MAX_CHAVES + 1, fileIndice) != (size_t)(NRO_MAX_CHAVES + 1)) return false;
    return true;
}

bool encontrarChave(No *no, int chave, int *subArvore, int *ponteiroDados){
    int i = 0;

    // busca sequencial dentro do nó, já que o número máximo de chaves é pequeno (3)
    while (i < no->nroChaves && no->C[i] != -1 && chave > no->C[i]) i++;

    // se a chave for encontrada, retorna o ponteiro de dados. Caso contrário, retorna o ponteiro da subárvore correspondente para continuar a busca recursiva.
    if (i < no->nroChaves && no->C[i] == chave) {
        if (ponteiroDados) *ponteiroDados = no->Pr[i];
        return true;
    }

    // se a chave não for encontrada, o ponteiro da subárvore é determinado pelo índice i, que indica a posição onde a chave deveria estar. Se i for igual ao número de chaves, a subárvore correta é a última (P[nroChaves]). Caso contrário, é a subárvore P[i].
    if (subArvore) *subArvore = no->P[i];
    return false;
}

bool insereOrdenado(No *no, int posicao, int chave, int ponteiroDados, int filhoEsq, int filhoDir){
    int i;

    // realiza o shift para abrir espaço para a nova chave, ponteiro de dados e ponteiros de filhos
    for (i = no->nroChaves; i > posicao; i--) {
        no->C[i] = no->C[i - 1];
        no->Pr[i] = no->Pr[i - 1];
    }

    // shift dos ponteiros de filhos
    for (i = no->nroChaves + 1; i > posicao + 1; i--) {
        no->P[i] = no->P[i - 1];
    }

    // insere a nova chave, ponteiro de dados e ponteiros de filhos na posição correta
    no->C[posicao] = chave; // chave a ser inserida
    no->Pr[posicao] = ponteiroDados; // ponteiro de dados associado à chave
    no->P[posicao] = filhoEsq;  // ponteiro do filho esquerdo (pode ser -1 se for folha)
    no->P[posicao + 1] = filhoDir; // ponteiro do filho direito (pode ser -1 se for folha)
    no->nroChaves++;

    return true;
}

int comparaInt(const void *a, const void *b) {
    int n1 = *(const int *)a;
    int n2 = *(const int *)b;
    return (n1 > n2) - (n1 < n2);
}

/* (ideia de implementação) */
int distribuirOrdenado(No *no, No *novoNo, int posicaoInsercao, int chaveNova, int ponteiroDadosNova, int filhoEsqNova, int filhoDirNova, int *ponteiroDadosPromovido){
    // cria arrays temporários para armazenar as chaves, ponteiros de dados e ponteiros de filhos do nó original mais a nova chave/ponteiro/filhos a serem inseridos
    int chavesTemp[NRO_MAX_CHAVES + 1];
    int ponteirosTemp[NRO_MAX_CHAVES + 1];
    int filhosTemp[NRO_MAX_CHAVES + 2];
    int totalChaves, indicePromocao, i, tipoOriginal;

    totalChaves = no->nroChaves + 1; // total de chaves considerando a nova chave a ser inserida

    // inicializa os arrays temporários com valores inválidos (-1) para facilitar a manipulação
    for (i = 0; i < NRO_MAX_CHAVES + 1; i++) {
        chavesTemp[i] = -1;
        ponteirosTemp[i] = -1;
    }

    // inicializa o array de ponteiros de filhos com -1, indicando que inicialmente não há filhos
    for (i = 0; i < NRO_MAX_CHAVES + 2; i++) {
        filhosTemp[i] = -1;
    }

    tipoOriginal = no->tipoNo; // armazena o tipo original do nó para manter a consistência após a distribuição (se for raiz, continua sendo raiz; se for intermediário, continua sendo intermediário)

    // preenche os arrays temporários com as chaves, ponteiros de dados e ponteiros de filhos do nó original, inserindo a nova chave/ponteiro/filhos na posição correta
    for (i = 0; i < no->nroChaves; i++) {
        chavesTemp[i] = no->C[i];
        ponteirosTemp[i] = no->Pr[i];
    }

    // preenche o array de ponteiros de filhos do nó original
    for (i = 0; i < no->nroChaves + 1; i++) {
        filhosTemp[i] = no->P[i];
    }

    // realiza o shift para abrir espaço para a nova chave, ponteiro de dados e ponteiros de filhos na posição correta
    for (i = totalChaves - 1; i > posicaoInsercao; i--) {
        chavesTemp[i] = chavesTemp[i - 1];
        ponteirosTemp[i] = ponteirosTemp[i - 1];
    }

    // shift dos ponteiros de filhos
    for (i = totalChaves; i > posicaoInsercao + 1; i--) {
        filhosTemp[i] = filhosTemp[i - 1];
    }

    chavesTemp[posicaoInsercao] = chaveNova; // chave a ser inserida
    ponteirosTemp[posicaoInsercao] = ponteiroDadosNova; // ponteiro de dados associado à chave
    filhosTemp[posicaoInsercao] = filhoEsqNova; // ponteiro do filho esquerdo (pode ser -1 se for folha)
    filhosTemp[posicaoInsercao + 1] = filhoDirNova; // ponteiro do filho direito (pode ser -1 se for folha)

    indicePromocao = totalChaves / 2; // índice da chave a ser promovida para o nó pai (chave do meio)
    if (ponteiroDadosPromovido) *ponteiroDadosPromovido = ponteirosTemp[indicePromocao]; // ponteiro de dados associado à chave promovida, necessário para manter a consistência do índice

    // a chave do meio (índice de promoção) é promovida para o nó pai, as chaves à esquerda do índice de promoção permanecem no nó original e as chaves à direita do índice de promoção são movidas para o novo nó criado
    limparNo(no);
    limparNo(novoNo);

    no->removido = '0';
    no->proximo = -1;
    no->tipoNo = tipoOriginal;
    no->nroChaves = indicePromocao;

    novoNo->removido = '0';
    novoNo->proximo = -1;
    novoNo->tipoNo = tipoOriginal;
    novoNo->nroChaves = totalChaves - indicePromocao - 1;

    for (i = 0; i < no->nroChaves; i++) {
        no->C[i] = chavesTemp[i];
        no->Pr[i] = ponteirosTemp[i];
    }

    for (i = 0; i < no->nroChaves + 1; i++) {
        no->P[i] = filhosTemp[i];
    }

    for (i = no->nroChaves; i < NRO_MAX_CHAVES; i++) {
        no->C[i] = -1;
        no->Pr[i] = -1;
    }

    for (i = no->nroChaves + 1; i < NRO_MAX_CHAVES + 1; i++) {
        no->P[i] = -1;
    }

    for (i = 0; i < novoNo->nroChaves; i++) {
        novoNo->C[i] = chavesTemp[indicePromocao + 1 + i];
        novoNo->Pr[i] = ponteirosTemp[indicePromocao + 1 + i];
    }

    for (i = 0; i < novoNo->nroChaves + 1; i++) {
        novoNo->P[i] = filhosTemp[indicePromocao + 1 + i];
    }

    for (i = novoNo->nroChaves; i < NRO_MAX_CHAVES; i++) {
        novoNo->C[i] = -1;
        novoNo->Pr[i] = -1;
    }

    for (i = novoNo->nroChaves + 1; i < NRO_MAX_CHAVES + 1; i++) {
        novoNo->P[i] = -1;
    }

    return chavesTemp[indicePromocao];
}

void apagarNo(FILE *fileIndice, int rrnNoParaApagar) {
    int topoAtual;
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
    fread(&topoAtual, sizeof(int), 1, fileIndice);

    // Altera apenas o removido e o encadeamento (mantém os bytes antigos intactos)
    fseek(fileIndice, TAM_BTREE_CABECALHO + rrnNoParaApagar * TAM_NO, SEEK_SET);
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