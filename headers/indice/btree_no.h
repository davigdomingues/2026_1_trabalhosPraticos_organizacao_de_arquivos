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

typedef struct {
    int chave;
    int ptrDados;
    int filhoDir;
} ElementoIndice;

No *inicializarNo();
void lerNo(FILE *fileIndice, No *no);
bool encontrarChave(No *no, int chave, int *subArvore, int *ponteiroDados);
bool insereOrdenado(No *no, ElementoIndice elem);
bool escreverNo(FILE *file, No *no);
No *criarNo(FILE *fileIndice, int *novoRRN);
void distribuirUniforme(FILE *fileIndice, No *no, No *novoNo, int rrnNovoNo, ElementoIndice overflowElem, ElementoIndice *promoPraCima);
void apagarNo(FILE *fileIndice, int rrnNoParaApagar);

#endif