#include <stdio.h>
#define TAM_REG 80

typedef struct Registro {
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distanciaProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    int tamNomeLinha;
    char *nomeEstacao;
    char *nomeLinha;
    char removido;
} Registro;

void insert(FILE *file, int codEstacaoInput, char *nomeEstacaoInput, char *codLinhaInput, char *nomeLinhaInput, char *codProxEstInput, char *distanciaProxEstInput, char *codLinhaIntegInput, char *codEstacaoIntegInput);