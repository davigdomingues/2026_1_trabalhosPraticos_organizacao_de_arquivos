#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/utils.h"
#include "../headers/indice/btree_no.h"

int freeMapKeys(const void* key, size_t ksize, uintptr_t value, void* usr){
    free((void*) key);
    return 0;
}

// considera como nulo um valor que seja NULL, ou uma string vazia, ou "void", ou "empty"
bool valorEhNulo(const char *valor) {
    if (valor == NULL) return true;
    return (*valor == '\0' || strcmp(valor, "void") == 0 || strcmp(valor, "empty") == 0);
}

// encontra o índice de um campo em um array de pares campo-valor, ou -1 se não encontrado
int encontrarIndexCampo(CampoValor *pares, int mPares, const char *campo) {
    for (int i = 0; i < mPares; i++) {
        if (strcmp(pares[i].campo, campo) == 0) return i;
    }
    return -1;
}

static int campoRegistroId(char *campo) {
    if (!campo) return -1;

    // tabela de mapeamento nome -> id (ordem fixa do registro)
    static const struct { const char *nome; int id; } mapa[] = {
        { "codEstacao", CAMPO_COD_ESTACAO },
        { "nomeEstacao", CAMPO_NOME_ESTACAO },
        { "codLinha", CAMPO_COD_LINHA },
        { "nomeLinha", CAMPO_NOME_LINHA },
        { "codProxEstacao", CAMPO_COD_PROX_ESTACAO },
        { "distProxEstacao", CAMPO_DIST_PROX_ESTACAO },
        { "codLinhaIntegra", CAMPO_COD_LINHA_INTEGRA },
        { "codEstIntegra", CAMPO_COD_EST_INTEGRA },

        // tolerância a um typo que aparece em algumas implementações
        { "codEstacaoIntegra", CAMPO_COD_EST_INTEGRA },
    };

    // busca via hashmap
    int length = (int)(sizeof(mapa) / sizeof(mapa[0]));
    for (size_t i = 0; i < length; i++) {
        if (strcmp(campo, mapa[i].nome) == 0) {
            return mapa[i].id;
        }
    }

    return -1;
}

// preenche um array (tamanho NUM_CAMPOS_REGISTRO) com ponteiros para os pares encontrados
// posições sem filtro ficam como NULL e retorna quantos campos reconhecidos foram preenchidos
int popularParesPorCampo(CampoValor *pares, int mPares, CampoValor *out[NUM_CAMPOS_REGISTRO]) {
    if (!out) return 0;
    for (int i = 0; i < NUM_CAMPOS_REGISTRO; i++) out[i] = NULL; // inicializa todas as posições como NULL

    if (!pares || mPares <= 0) return 0; // sem pares para processar

    int preenchidos = 0;
    for (int i = 0; i < mPares; i++) { // para cada par, encontra o id do campo e, se for reconhecido, armazena o ponteiro na posição correspondente do array de saída
        int id = campoRegistroId(pares[i].campo);
        if (id < 0) continue;
        if (out[id] == NULL) preenchidos++; // conta apenas o primeiro par encontrado para cada campo
        out[id] = &pares[i]; // armazena o ponteiro para o par encontrado na posição correspondente do array de saída
    }

    return preenchidos;
}


// aloca e seta uma string do Registro a partir de um valor de par campo-valor (trata NULO e libera string anterior quando necessário)
static bool setRegistroString(char **dest, int *destTam, char *valor) {
    if (!dest || !destTam) return true;

    if (*destTam > 0 && *dest) {
        free(*dest);
    }

    if (valorEhNulo(valor)) { // trata valor nulo: seta string vazia e tamanho 0
        *dest = "";
        *destTam = 0;
        return true;
    }

    // aloca nova string e seta valor e tamanho
    int novoTam = (int)strlen(valor);
    char *novo = (char*) malloc((size_t)novoTam + 1);
    if (!novo) return false;
    memcpy(novo, valor, (size_t)novoTam + 1);
    *dest = novo;
    *destTam = novoTam;
    return true;
}

// aplica um par campo-valor em um Registro (converte int, trata NULO e aloca strings quando necessário)
// retorna false apenas em falha de alocação
bool aplicarParEmRegistro(Registro *reg, CampoValor *par) {
    if (!reg || !par) return true;

    // identifica o campo do par e seu valor
    char *campo = par->campo;
    char *valor = par->valor;
    int id = campoRegistroId(campo);
    if (id < 0) return true; // campo desconhecido: ignora

    // aplica o valor do par no campo correspondente do registro, convertendo para int quando necessário e tratando valores nulos
    switch ((CampoRegistroId)id) {
        case CAMPO_COD_ESTACAO:
            reg->codEstacao = valorEhNulo(valor) ? -1 : atoi(valor);
            return true;
        case CAMPO_COD_LINHA:
            reg->codLinha = valorEhNulo(valor) ? -1 : atoi(valor);
            return true;
        case CAMPO_COD_PROX_ESTACAO:
            reg->codProxEstacao = valorEhNulo(valor) ? -1 : atoi(valor);
            return true;
        case CAMPO_DIST_PROX_ESTACAO:
            reg->distProxEstacao = valorEhNulo(valor) ? -1 : atoi(valor);
            return true;
        case CAMPO_COD_LINHA_INTEGRA:
            reg->codLinhaIntegra = valorEhNulo(valor) ? -1 : atoi(valor);
            return true;
        case CAMPO_COD_EST_INTEGRA:
            reg->codEstIntegra = valorEhNulo(valor) ? -1 : atoi(valor);
            return true;
        case CAMPO_NOME_ESTACAO:
            return setRegistroString(&reg->nomeEstacao, &reg->tamNomeEstacao, valor);
        case CAMPO_NOME_LINHA:
            return setRegistroString(&reg->nomeLinha, &reg->tamNomeLinha, valor);
        default:
            return true;
    }
}

// aplicação de vários pares em sequência
bool aplicarParesEmRegistro(Registro *reg, CampoValor *pares, int mPares) {
    if (!reg) return true;
    if (!pares || mPares <= 0) return true;
    for (int i = 0; i < mPares; i++) { // aplica cada par em sequência e, se houver falha de alocação, retorna false imediatamente
        if (!aplicarParEmRegistro(reg, &pares[i])) return false;
    }
    return true;
}

void insertionSort(ElementoIndice arr[], int tam) {
    for (int i = 1; i < tam; i++) {
        ElementoIndice chaveAtual = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].chave > chaveAtual.chave) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = chaveAtual;
    }
}
