/*
    Integrantes:
    - Davi Gabriel Domingues (15447497)
    - Felipe Ferreira Colona (15636525)
*/

#include "../headers/dados/operacoes.h"
#include "../headers/fornecidas.h"
#include "../headers/dados/registro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TAM_CAMPO 20
#define TAM_VALOR 50
#define TAM_ARQUIVO 100 // tamanho máximo para nome de arquivo
#define MAX_PARES 8 // número máximo de pares

/** @brief lê os pares campo-valor da entrada padrão e armazená-los em um array de CampoValor
 * 
 * @param pares array de CampoValor a ser preenchido
 * @param mPares tamanho do array de pares
 */
static void lerPares(CampoValor *pares, int mPares) {
    for (int j = 0; j < mPares; j++) {
        char *campo = (char*) malloc(sizeof(char) * TAM_CAMPO);
        char *valor = (char*) malloc(sizeof(char) * TAM_VALOR);

        // evita overflow
        scanf("%19s", campo);

        int valorInt;
        if (scanf("%d", &valorInt) <= 0) {
            ScanQuoteString(valor);
        } else {
            // TAM_VALOR é o tamanho do buffer alocado
            snprintf(valor, TAM_VALOR, "%d", valorInt);
        }

        pares[j] = (CampoValor){.campo = campo, .valor = valor};
    }
}

/** @brief libera a memória alocada para os campos e os valores de um array de CampoValor
 * 
 * @param pares array de CampoValor a ter sua memória liberada
 * @param mPares tamanho do array de pares
 */
static void liberarPares(CampoValor *pares, int mPares) {
    for (int j = 0; j < mPares; j++) {
        free(pares[j].campo);
        free(pares[j].valor);
    }
}

/** @brief lê um nome de arquivo (token sem espaços) e devolve uma string alocada
 * 
 * @return ponteiro para a string alocada, ou NULL em caso de erro
 */
static char *lerNomeArquivo(void) {
    char *s = (char*) malloc(TAM_ARQUIVO); // aloca o máximo necessário para o nome do arquivo, incluindo o caractere nulo
    if (!s) return NULL;

    // uso de TAM_ARQUIVO - 1 para limitar a string, em que a forma direta é montada em runtime
    int width = TAM_ARQUIVO - 1;
    if (width < 1) { 
        free(s); 
        return NULL; 
    }

    // formatação para ler uma string sem espaços, limitada ao tamanho do buffer alocado
    char fmt[32];
    snprintf(fmt, sizeof(fmt), "%%%ds", width); // o %%%ds é usado para criar a formatação correta, resultando na leitura até TAM_ARQUIVO - 1 caracteres

    // se a leitura falhar, libera a memória e retorna NULL
    if (scanf(fmt, s) != 1) {
        free(s);
        return NULL;
    }
    return s;
}

int main(){
    int op;
    scanf("%d", &op);

    char *arquivoDados = NULL;
    char *arquivoSaida = NULL;
    char *arquivoIndice = NULL;
    bool ok = false;
    switch (op) {
        case 1: // CREATE
            arquivoDados = lerNomeArquivo();
            arquivoSaida   = lerNomeArquivo();
            if (!arquivoDados || !arquivoSaida) return -1;

            ok = create(arquivoDados, arquivoSaida);
            if(ok) BinarioNaTela(arquivoSaida);
            break;
        case 2: // SELECT ALL
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados) return -1;

            selectAll(arquivoDados);
            break;
        case 3: // SELECT ALL WHERE
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados) return -1;

            int nBuscas = 0;
            scanf("%d", &nBuscas);

            CampoValor *pares = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            for (int i = 0; i < nBuscas; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                lerPares(pares, mPares);

                selectAllWhere(arquivoDados, NULL, pares, mPares);

                liberarPares(pares, mPares);
            }
            free(pares);
            break;
        case 4: // DELETE WHERE
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados) return -1;

            int nRemocoes = 0; // número de operações de remoção a serem realizadas
            scanf("%d", &nRemocoes);

            CampoValor *paresDelete = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES); // pares da remoção atual
            ok = true;
            // loop operações de deleção
            for (int i = 0; i < nRemocoes; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                lerPares(paresDelete, mPares); // leitura dos pares para a operação de remoção atual

                if (ok && !deleteWhere(arquivoDados, paresDelete, mPares)) {
                    ok = false; // como falhou, não tenta as próximas
                }

                liberarPares(paresDelete, mPares); // liberação dos pares da operação de remoção atual antes de ler os próximos, para evitar acúmulo de memória alocada
            }

            free(paresDelete);

            if (ok) BinarioNaTela(arquivoDados);
            break;
        case 5: // INSERT
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados) return -1;

            int nInsercoes = 0; // número de operações de inserção a serem realizadas
            scanf("%d", &nInsercoes);

            ok = true; // se houver falha em alguma das inserções, ok = false e as próximas inserções não são tentadas

            // cada inserção inclui os valores de todos os campos do registro, mesmo que sejam nulos
            for (int i = 0; i < nInsercoes; i++) {
                CampoValor valores[MAX_PARES];
                char valoresStr[MAX_PARES][LIMITE];
                valores[0].campo = "codEstacao";
                valores[1].campo = "nomeEstacao";
                valores[2].campo = "codLinha";
                valores[3].campo = "nomeLinha";
                valores[4].campo = "codProxEstacao";
                valores[5].campo = "distProxEstacao";
                valores[6].campo = "codLinhaIntegra";
                valores[7].campo = "codEstacaoIntegra";

                // lê os valores como string, mesmo os inteiros, para padronizar
                // se o valor for nulo, salva como string vazia
                for (int k = 0; k < MAX_PARES; k++) {
                    valores[k].valor = valoresStr[k];
                    ScanQuoteString(valores[k].valor);
                }

                // se houver falha na inserção, ok = false e as próximas inserções não são tentadas
                if (ok && !insert(arquivoDados, arquivoIndice, valores, MAX_PARES)) ok = false;
            }

            if (ok) BinarioNaTela(arquivoDados);
            break;
        case 6: // UPDATE
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados) return -1;

            int nAtualizacoes = 0; // número de operações de atualização a serem realizadas
            scanf("%d", &nAtualizacoes);

            // busca e atualização (pares campo-valor)
            CampoValor *paresBusca = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            CampoValor *paresUpdate = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            
            // loop das operações de atualização
            bool okUpdate = true;

            for (int i = 0; i < nAtualizacoes; i++) {
                int mParesBusca = 0;
                scanf("%d", &mParesBusca);

                lerPares(paresBusca, mParesBusca); // leitura dos pares para a parte de busca da operação de atualização atual

                int mParesUpdate = 0; // número de pares para a parte de atualização da operação de atualização atual
                scanf("%d", &mParesUpdate);

                lerPares(paresUpdate, mParesUpdate);

                // se houver falha em alguma das atualizações, okUpdate = false e as próximas atualizações não são tentadas
                if (okUpdate && !update(arquivoDados, arquivoDados, paresBusca, mParesBusca, paresUpdate, mParesUpdate)) {
                    okUpdate = false; // falha real
                }

                // liberação dos pares da operação de atualização atual antes de ler os próximos
                liberarPares(paresBusca, mParesBusca);
                liberarPares(paresUpdate, mParesUpdate);

                if (!okUpdate) break; // encerra antes de completar as N atualizações
            }

            free(paresBusca);
            free(paresUpdate);

            if (okUpdate) BinarioNaTela(arquivoDados);

            break;
        case 8: //SELECT WHERE COM INDEXAÇÃO
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados) return -1;

            arquivoIndice = lerNomeArquivo();
            if (!arquivoIndice) return -1;

            nBuscas = 0;
            scanf("%d", &nBuscas);

            pares = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            for (int i = 0; i < nBuscas; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                lerPares(pares, mPares);

                selectAllWhere(arquivoDados, arquivoIndice, pares, mPares);

                liberarPares(pares, mPares);
            }
            free(pares);
            break;
        case 10:
            arquivoDados = lerNomeArquivo();
            if (!arquivoDados) return -1;

            arquivoIndice = lerNomeArquivo();
            if (!arquivoIndice) return -1;

            nInsercoes = 0; // número de operações de inserção a serem realizadas
            scanf("%d", &nInsercoes);

            ok = true; // se houver falha em alguma das inserções, ok = false e as próximas inserções não são tentadas

            // cada inserção inclui os valores de todos os campos do registro, mesmo que sejam nulos
            for (int i = 0; i < nInsercoes; i++) {
                CampoValor valores[MAX_PARES];
                char valoresStr[MAX_PARES][LIMITE];
                valores[0].campo = "codEstacao";
                valores[1].campo = "nomeEstacao";
                valores[2].campo = "codLinha";
                valores[3].campo = "nomeLinha";
                valores[4].campo = "codProxEstacao";
                valores[5].campo = "distProxEstacao";
                valores[6].campo = "codLinhaIntegra";
                valores[7].campo = "codEstacaoIntegra";

                // lê os valores como string, mesmo os inteiros, para padronizar
                // se o valor for nulo, salva como string vazia
                for (int k = 0; k < MAX_PARES; k++) {
                    valores[k].valor = valoresStr[k];
                    ScanQuoteString(valores[k].valor);
                }

                // se houver falha na inserção, ok = false e as próximas inserções não são tentadas
                if (ok && !insert(arquivoDados, arquivoIndice, valores, MAX_PARES)) ok = false;
            }

            if (ok) BinarioNaTela(arquivoDados);
            break;
        default: // operação inválida
            return -1;
    }
    free(arquivoDados);
    free(arquivoIndice);
    free(arquivoSaida);

    return 0;
}