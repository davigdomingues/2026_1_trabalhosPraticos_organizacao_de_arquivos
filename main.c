#include "operacoes.h"
#include "cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "fornecidas.h"
#include "registro.h"

// tamanho fixo (em bytes) que um registro ocupa sem os dois campos string.
// layout em registro.c: 1 char removido + 9 inteiros (inclui tamNomeEstacao e tamNomeLinha)
#define TAM_FIXO_REG ((int)sizeof(char) + 9 * (int)sizeof(int))

// LIMITE é a capacidade (inclui '\0') de um buffer para armazenar um campo string lido da entrada
// no pior caso, um dos campos (nomeEstacao ou nomeLinha) pode ocupar todo o espaço variável do registro.
#define LIMITE (TAM_REG - TAM_FIXO_REG + 1)

static bool lerStatusCabecalho(const char *nomeArquivo, char *statusOut) {
    FILE *f = fopen(nomeArquivo, "rb");
    if (!f) return false;
    char status;
    if (fread(&status, sizeof(char), 1, f) != 1) {
        fclose(f);
        return false;
    }
    fclose(f);
    *statusOut = status;
    return true;
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

                for (int j = 0; j < mPares; j++) {
                    char *campo = (char*) malloc(sizeof(char) * 20);
                    char *valor = (char*) malloc(sizeof(char) * 50);
                    scanf("%s", campo);

                    int valorInt;
                    //se o valor não for um inteiro
                    if(scanf("%d", &valorInt) <= 0) {
                        //pega a string entre aspas
                        ScanQuoteString(valor);
                    } else {
                        //se for inteiro, salva como string para padronizar
                        snprintf(valor, sizeof(valor), "%d", valorInt);
                    }

                    CampoValor *par = malloc(sizeof(CampoValor));
                    par->campo = campo;
                    par->valor = valor;
                    pares[j] = *par;
                }

                int *rrns;
                selectWhere(arquivoEntrada, pares, mPares, &rrns, true);

                for (int j = 0; j < mPares; j++) {
                    free(pares[j].campo);
                    free(pares[j].valor);
                }
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

                for (int j = 0; j < mPares; j++) {
                    char *campo = (char*) malloc(sizeof(char) * 20);
                    char *valor = (char*) malloc(sizeof(char) * 50);
                    scanf("%s", campo);

                    int valorInt;
                    //se o valor não for um inteiro
                    if(scanf("%d", &valorInt) <= 0) {
                        //pega a string entre aspas
                        ScanQuoteString(valor);
                    } else {
                        //se for inteiro, salva como string para padronizar
                        snprintf(valor, sizeof(valor), "%d", valorInt);
                    }

                    paresDelete[j] = (CampoValor){.campo = campo, .valor = valor};
                }

                if (ok && !deleteWhere(arquivoEntrada, paresDelete, mPares)) {
                    ok = false;
                }

                for (int j = 0; j < mPares; j++) {
                    free(paresDelete[j].campo);
                    free(paresDelete[j].valor);
                }
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

                for (int j = 0; j < mParesBusca; j++) {
                    char *campo = (char*) malloc(sizeof(char) * 20);
                    char *valor = (char*) malloc(sizeof(char) * 50);
                    scanf("%s", campo);

                    int valorInt;
                    if(scanf("%d", &valorInt) <= 0) {
                        ScanQuoteString(valor);
                    } else {
                        snprintf(valor, sizeof(valor), "%d", valorInt);
                    }

                    paresBusca[j] = (CampoValor){.campo = campo, .valor = valor};
                }

                int mParesUpdate = 0;
                scanf("%d", &mParesUpdate);

                for (int j = 0; j < mParesUpdate; j++) {
                    char *campo = (char*) malloc(sizeof(char) * 20);
                    char *valor = (char*) malloc(sizeof(char) * 50);
                    scanf("%s", campo);

                    int valorInt;
                    if(scanf("%d", &valorInt) <= 0) {
                        ScanQuoteString(valor);
                    } else {
                        snprintf(valor, sizeof(valor), "%d", valorInt);
                    }

                    paresUpdate[j] = (CampoValor){.campo = campo, .valor = valor};
                }

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

                for (int j = 0; j < mParesBusca; j++) {
                    free(paresBusca[j].campo);
                    free(paresBusca[j].valor);
                }

                for (int j = 0; j < mParesUpdate; j++) {
                    free(paresUpdate[j].campo);
                    free(paresUpdate[j].valor);
                }

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