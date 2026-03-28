#include <stdio.h>
#define TAM_REG 80

typedef struct Registro {
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    int tamNomeLinha;
    char *nomeEstacao;
    char *nomeLinha;
    char removido;
} Registro;

void escreverReg(FILE *file, Registro *reg);
void printReg(Registro *reg);
