#ifndef REGISTRO_H
#define REGISTRO_H

#include <stdbool.h>
#include <stdio.h>

#define TAM_REG 80 // tamanho fixo (em bytes) que um registro ocupa no arquivo

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


/**
 * @brief Inicializa um registro em memória estaticamente.
 * 
 * @return Registro registro inicializado
 */
Registro inicializarReg();


/**
 * @brief Escreve um registro em um arquivo aberto, campo a campo.
 * 
 * @param file arquivo aberto
 * @param reg registro a ser escrito
 */
void escreverReg(FILE *file, Registro *reg);


/**
 * @brief Imprime um registro na tela.
 * 
 * @param reg registro a ser impresso
 */
void printReg(Registro *reg);

/** @brief Verifica se uma estação já existe em um arquivo.
 * 
 * @param file arquivo aberto
 * @param nomeEstacao nome da estação a ser verificada
 * @param tamNomeEstacao tamanho do nome da estação
 * @return true se a estação já existir
 * @return false se a estação não existir
 */
bool nomeEstacaoJaExiste(FILE *file, const char *nomeEstacao, int tamNomeEstacao);

#endif
