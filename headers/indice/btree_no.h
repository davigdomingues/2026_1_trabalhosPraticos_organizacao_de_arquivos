#ifndef NO_H
#define NO_H

#include <stdbool.h>
#include <stdio.h>

#define TAM_NO 53
#define NO_FOLHA -1 
#define NO_RAIZ 0 
#define NO_INTERMEDIARIO 1 
#define NO_NAO_INICIALIZADO 10
#define NRO_MAX_CHAVES 3

typedef struct No {
    int proximo;
    int tipoNo;
    int nroChaves;
    int C[3];
    int Pr[3];
    int P[4];
    char removido;
} No;

No *incializarNo();
void lerNo(FILE *fileIndice, No *no);
bool encontrarChave(No *no, int chave, int *subArvore, int *ponteiroDados);
bool insereOrdenado(No *no, int posicao, int chave, int ponteiroDados, int filhoEsq, int filhoDir); // mudou aqui
bool escreverNo(FILE *file, No *no);
No *criarNo(FILE *fileIndice, int *novoRRN);
int distribuirOrdenado(No *no, No *novoNo, int posicaoInsercao, int chaveNova, int ponteiroDadosNova, int filhoEsqNova, int filhoDirNova, int *ponteiroDadosPromovido); // mudou aqui
void apagarNo(FILE *fileIndice, int rrnNoParaApagar);

#endif