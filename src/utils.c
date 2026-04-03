#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/utils.h"

int freeMapKeys(const void* key, size_t ksize, uintptr_t value, void* usr){
    free((void*) key);
    return 0;
}

bool verificarMatchInt(int index, char *valorQuery, int valorReg) {
    //a query inclui um valor a ser buscado para esse campo?
    if (index > -1) {
        //o valor a ser buscado para esse campo é nulo e o valor do registro atual também é?
        //ou o valor a ser buscado para esse campo não é nulo e é igual ao valor do registro atual?
        if ((*valorQuery == '\0' && valorReg == -1) || atoi(valorQuery) == valorReg) {
            //se sim, houve um match
            return true;
        }
    }
    //se não, não houve match
    return false;
}

bool verificarMatchStr(int index, char *valorQuery, char *valorReg) {
    //a query inclui um valor a ser buscado para esse campo
    //e os valores são iguais?
    if (index > -1 && strcmp(valorQuery, valorReg) == 0) { 
        //OBS: a comparação também funciona pros casos nulos
        //porque, simplesmente, a comparação é feita com strings vazias
        return true;
    }
    //se não, não houve match
    return false;
}

bool valorEhNulo(const char *valor) {
    if (valor == NULL) return true;
    return (*valor == '\0' || strcmp(valor, "void") == 0 || strcmp(valor, "empty") == 0);
}

bool registroMatchParBusca(const Registro *reg, const char *nomeEstacao, const char *nomeLinha, const CampoValor *par) {
    const char *campo = par->campo;
    const char *valor = par->valor;

    if (strcmp(campo, "codEstacao") == 0) {
        return valorEhNulo(valor) ? (reg->codEstacao == -1) : (atoi(valor) == reg->codEstacao);
    }

    if (strcmp(campo, "codLinha") == 0) {
        return valorEhNulo(valor) ? (reg->codLinha == -1) : (atoi(valor) == reg->codLinha);
    }

    if (strcmp(campo, "codProxEstacao") == 0) {
        return valorEhNulo(valor) ? (reg->codProxEstacao == -1) : (atoi(valor) == reg->codProxEstacao);
    }

    if (strcmp(campo, "distProxEstacao") == 0) {
        return valorEhNulo(valor) ? (reg->distProxEstacao == -1) : (atoi(valor) == reg->distProxEstacao);
    }

    if (strcmp(campo, "codLinhaIntegra") == 0) {
        return valorEhNulo(valor) ? (reg->codLinhaIntegra == -1) : (atoi(valor) == reg->codLinhaIntegra);
    }

    if (strcmp(campo, "codEstIntegra") == 0) {
        return valorEhNulo(valor) ? (reg->codEstIntegra == -1) : (atoi(valor) == reg->codEstIntegra);
    }

    if (strcmp(campo, "nomeEstacao") == 0 || strcmp(campo, "nomeEstcao") == 0) {
        return valorEhNulo(valor) ? (reg->tamNomeEstacao == 0) : (strcmp(valor, nomeEstacao) == 0);
    }

    if (strcmp(campo, "nomeLinha") == 0) {
        return valorEhNulo(valor) ? (reg->tamNomeLinha == 0) : (strcmp(valor, nomeLinha) == 0);
    }

    return false;
}


int encontrarIndexCampo(CampoValor *pares, int mPares, const char *campo) {
    for (int i = 0; i < mPares; i++) {
        if (strcmp(pares[i].campo, campo) == 0) return i;
    }
    return -1;
}

