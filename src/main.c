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

int main(){
    int op;
    scanf("%d", &op);

    char *arquivoEntrada = NULL;
    char *arquivoSaida = NULL;
    bool ok = false;
    switch (op) {
        case 1:
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            arquivoSaida = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);
            scanf("%s", arquivoSaida);

            ok = create(arquivoEntrada, arquivoSaida);
            if(ok) BinarioNaTela(arquivoSaida);
            break;
        case 2:
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);

            selectAll(arquivoEntrada);
            break;
        case 3:
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);
            int nBuscas = 0;
            scanf("%d", &nBuscas);

            CampoValor *pares = (CampoValor*) malloc(sizeof(CampoValor) * 8); //8 é o número máximo de pares
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
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);

            int nRemocoes = 0;
            scanf("%d", &nRemocoes);

            CampoValor *paresDelete = (CampoValor*) malloc(sizeof(CampoValor) * 8);
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
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);

            int nInsercoes = 0;
            scanf("%d", &nInsercoes);

            ok = true;

            // cada inserção inclui os valores de todos os campos do registro, mesmo que sejam nulos
            for (int i = 0; i < nInsercoes; i++) {
                CampoValor valores[8];
                char valoresStr[8][LIMITE];
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
                for (int k = 0; k < 8; k++) {
                    valores[k].valor = valoresStr[k];
                    ScanQuoteString(valores[k].valor);
                }

                // se houver falha na inserção, ok = false e as próximas inserções não são tentadas
                if (ok && !insert(arquivoEntrada, valores, 8)) ok = false;
            }

            if (ok) BinarioNaTela(arquivoEntrada);
            break;
        case 6:
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);

            int nAtualizacoes = 0;
            scanf("%d", &nAtualizacoes);

            CampoValor *paresBusca = (CampoValor*) malloc(sizeof(CampoValor) * 8);
            CampoValor *paresUpdate = (CampoValor*) malloc(sizeof(CampoValor) * 8);
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