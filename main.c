#include "operacoes.h"
#include "cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "fornecidas.h"

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