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
    switch (op) {
        case 1:
            arquivoEntrada = (char*) malloc(sizeof(char) * 100);
            arquivoSaida = (char*) malloc(sizeof(char) * 100);
            scanf("%s", arquivoEntrada);
            scanf("%s", arquivoSaida);

            create(arquivoEntrada, arquivoSaida);
            BinarioNaTela(arquivoSaida);
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
                int tamResultados;
                int *rrns;
                Registro *resultados = selectWhere(arquivoEntrada, pares, mPares, &tamResultados, &rrns);
                for (int i = 0; i < tamResultados; i++) {
                    printReg(&resultados[i]);
                }

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
            bool ok = true;
            for (int i = 0; i < nRemocoes; i++) {
                int mPares = 0;
                scanf("%d", &mPares);

                for (int j = 0; j < mPares; j++) {
                    char *campo = (char*) malloc(sizeof(char) * 20);
                    char *valor = (char*) malloc(sizeof(char) * 50);
                    scanf("%s", campo);
                    ScanQuoteString(valor);

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
        default:
            return -1;
    }
    free(arquivoEntrada);
    free(arquivoSaida);

    return 0;
}