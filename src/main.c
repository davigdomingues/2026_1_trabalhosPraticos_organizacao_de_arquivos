#include "../headers/operacoes.h"
#include "../headers/fornecidas.h"
#include "../headers/cabecalho.h"
#include "../headers/registro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TAM_CAMPO 20
#define TAM_VALOR 50
#define TAM_ARQUIVO 100 // tamanho máximo para nome de arquivo
#define MAX_PARES 8 // número máximo de pares

// função para ler os pares campo-valor da entrada padrão e armazená-los em um array de CampoValor
static void lerPares(CampoValor *pares, int mPares) {
    for (int j = 0; j < mPares; j++) {
        char *campo = (char*) malloc(sizeof(char) * TAM_CAMPO);
        char *valor = (char*) malloc(sizeof(char) * TAM_VALOR);
        scanf("%s", campo);

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

// função para liberar a memória alocada para os campos e valores de um array de CampoValor
static void liberarPares(CampoValor *pares, int mPares) {
    for (int j = 0; j < mPares; j++) {
        free(pares[j].campo);
        free(pares[j].valor);
    }
}

// lê um nome de arquivo (token sem espaços) e devolve uma string alocada
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

    char *arquivoEntrada = NULL;
    char *arquivoSaida = NULL;
    bool ok = false;
    switch (op) {
        case 1:
            arquivoEntrada = lerNomeArquivo();
            arquivoSaida   = lerNomeArquivo();
            if (!arquivoEntrada || !arquivoSaida) return -1;

            ok = create(arquivoEntrada, arquivoSaida);
            if(ok) BinarioNaTela(arquivoSaida);
            break;
        case 2:
            arquivoEntrada = lerNomeArquivo();
            if (!arquivoEntrada) return -1;

            selectAll(arquivoEntrada);
            break;
        case 3:
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);
            int nBuscas = 0;
            scanf("%d", &nBuscas);

            CampoValor *pares = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            for (int i = 0; i < nBuscas; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                lerPares(pares, mPares);

                int *rrns;
                selectWhere(arquivoEntrada, pares, mPares, &rrns, true);

                liberarPares(pares, mPares);
            }
            free(pares);
            break;
        case 4:
            arquivoEntrada = lerNomeArquivo();
            if (!arquivoEntrada) return -1;

            int nRemocoes = 0;
            scanf("%d", &nRemocoes);

            CampoValor *paresDelete = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            ok = true;
            for (int i = 0; i < nRemocoes; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                lerPares(paresDelete, mPares);

                if (ok && !deleteWhere(arquivoEntrada, paresDelete, mPares)) {
                    ok = false;
                }

                liberarPares(paresDelete, mPares);
            }

            free(paresDelete);

            if (ok) BinarioNaTela(arquivoEntrada);
            break;
        case 5:
            arquivoEntrada = lerNomeArquivo();
            if (!arquivoEntrada) return -1;

            int nInsercoes = 0;
            scanf("%d", &nInsercoes);

            ok = true;

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
                if (ok && !insert(arquivoEntrada, valores, MAX_PARES)) ok = false;
            }

            if (ok) BinarioNaTela(arquivoEntrada);
            break;
        case 6:
            arquivoEntrada = lerNomeArquivo();
            if (!arquivoEntrada) return -1;

            int nAtualizacoes = 0;
            scanf("%d", &nAtualizacoes);

            CampoValor *paresBusca = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            CampoValor *paresUpdate = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
            bool okUpdate = true;
            bool encerrarCedoSemErro = false;
            for (int i = 0; i < nAtualizacoes; i++) {
                int mParesBusca = 0;
                scanf("%d", &mParesBusca);

                lerPares(paresBusca, mParesBusca);

                int mParesUpdate = 0;
                scanf("%d", &mParesUpdate);

                lerPares(paresUpdate, mParesUpdate);

                if (okUpdate) {
                    bool atualizou = update(arquivoEntrada, arquivoEntrada, paresBusca, mParesBusca, paresUpdate, mParesUpdate);
                    if (!atualizou) {
                        char statusCabecalho;
                        if (!lerStatusCabecalho(arquivoEntrada, &statusCabecalho) || statusCabecalho != '1') {
                            okUpdate = false; // falha real
                        } else {
                            encerrarCedoSemErro = true; // critério não satisfeito: encerra cedo e ainda imprime hash
                        }
                    }
                }

                liberarPares(paresBusca, mParesBusca);
                liberarPares(paresUpdate, mParesUpdate);

                if (!okUpdate || encerrarCedoSemErro) break; // encerra antes de completar as N atualizações
            }

            free(paresBusca);
            free(paresUpdate);

            if (okUpdate) BinarioNaTela(arquivoEntrada);

            break;
        default:
            return -1;
    }
    free(arquivoEntrada);
    free(arquivoSaida);

    return 0;
}