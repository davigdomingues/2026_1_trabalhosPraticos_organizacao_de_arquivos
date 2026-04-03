#ifndef REGISTRO_H
#define REGISTRO_H

#include <stdbool.h>
#include <stdio.h>

#define TAM_REG 80
// tamanho fixo (em bytes) que um registro ocupa sem os dois campos string.
// 1 char removido + 9 inteiros (inclui tamNomeEstacao e tamNomeLinha)
#define TAM_FIXO_REG ((int)sizeof(char) + 9 * (int)sizeof(int))

// LIMITE é a capacidade (inclui '\0') de um buffer para armazenar um campo string lido da entrada
// no pior caso, um dos campos (nomeEstacao ou nomeLinha) pode ocupar todo o espaço variável do registro.
#define LIMITE (TAM_REG - TAM_FIXO_REG + 1)

// TAM_LIVRE_REG é o espaço restante (pode ser 0) para preenchimento do registro, dado o tamanho atual de nomeEstacao e nomeLinha 
// valor calculado em runtime
#define TAM_LIVRE_REG(tamEst, tamLinha) (TAM_REG - TAM_FIXO_REG - tamEst - tamLinha)

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

Registro inicializarReg();
void escreverReg(FILE *file, Registro *reg);
void printReg(Registro *reg);
bool nomeEstacaoJaExiste(FILE *file, const char *nomeEstacao, int tamNomeEstacao);

#endif
