#include "../../headers/operacoes/buscas.h"
#include "../../headers/operacoes/insercoes.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/utils.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/indice/btree_no.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// verifica se um registro corresponde a um par campo-valor de busca, considerando os campos nomeEstacao e nomeLinha para comparação de strings
bool registroMatchParBusca(const Registro *reg, const char *nomeEstacao, const char *nomeLinha, const CampoValor *par) {
    char *campo = par->campo;
    char *valor = par->valor;

    // compara o valor do par com o campo correspondente do registro, tratando valores nulos e usando os parâmetros nomeEstacao e nomeLinha para comparação de strings
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


bool verificarMatchInt(int index, char *valorQuery, int valorReg) {
    //a query inclui um valor a ser buscado para esse campo?
    if (index > -1 && valorQuery != NULL) {
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
    if (index > -1 && valorQuery != NULL && valorReg != NULL && strcmp(valorQuery, valorReg) == 0) { 
        //OBS: a comparação também funciona pros casos nulos
        //porque, simplesmente, a comparação é feita com strings vazias
        return true;
    }
    //se não, não houve match
    return false;
}

int confereCriteriosBusca(FILE *fileDados, Registro *reg, CampoValor *porCampo[8], bool *codEstacaoMatch){
    if (!fileDados || !reg) return -1;

    // garante estado limpo
    reg->tamNomeEstacao = 0; reg->nomeEstacao = "";
    reg->tamNomeLinha   = 0; reg->nomeLinha   = "";

    int numMatches = 0;

    fseek(fileDados, 4, SEEK_CUR); //pula os 4 bytes de proxRRN

    fread(&reg->codEstacao, sizeof(int), 1, fileDados);
    if (porCampo[CAMPO_COD_ESTACAO] && verificarMatchInt(0, porCampo[CAMPO_COD_ESTACAO]->valor, reg->codEstacao)){
        *codEstacaoMatch = true;
        numMatches++;
    } else {
        *codEstacaoMatch = false;
    }

    fread(&reg->codLinha, sizeof(int), 1, fileDados);
    if (porCampo[CAMPO_COD_LINHA] && verificarMatchInt(0, porCampo[CAMPO_COD_LINHA]->valor, reg->codLinha)) numMatches++;

    fread(&reg->codProxEstacao, sizeof(int), 1, fileDados);
    if (porCampo[CAMPO_COD_PROX_ESTACAO] && verificarMatchInt(0, porCampo[CAMPO_COD_PROX_ESTACAO]->valor, reg->codProxEstacao)) numMatches++;

    fread(&reg->distProxEstacao, sizeof(int), 1, fileDados);
    if (porCampo[CAMPO_DIST_PROX_ESTACAO] && verificarMatchInt(0, porCampo[CAMPO_DIST_PROX_ESTACAO]->valor, reg->distProxEstacao)) numMatches++;

    fread(&reg->codLinhaIntegra, sizeof(int), 1, fileDados);
    if (porCampo[CAMPO_COD_LINHA_INTEGRA] && verificarMatchInt(0, porCampo[CAMPO_COD_LINHA_INTEGRA]->valor, reg->codLinhaIntegra)) numMatches++;

    fread(&reg->codEstIntegra, sizeof(int), 1, fileDados);
    if (porCampo[CAMPO_COD_EST_INTEGRA] && verificarMatchInt(0, porCampo[CAMPO_COD_EST_INTEGRA]->valor, reg->codEstIntegra)) numMatches++;

    fread(&reg->tamNomeEstacao, sizeof(int), 1, fileDados);
    //lê o nomeEstacao, se não for um campo NULO
    if(reg->tamNomeEstacao != 0){
        char *nomeEstacao = (char*) malloc(( sizeof(char) * reg->tamNomeEstacao ) + 1); // +1 para o caractere nulo
        if (!nomeEstacao) { 
            return -1; 
        }
        fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, fileDados);
        nomeEstacao[reg->tamNomeEstacao] = '\0';
        reg->nomeEstacao = nomeEstacao;
    } else {
        //se for NULO, só indica que é
        reg->nomeEstacao = "";
    }
    if (porCampo[CAMPO_NOME_ESTACAO] && verificarMatchStr(0, porCampo[CAMPO_NOME_ESTACAO]->valor, reg->nomeEstacao)) numMatches++;

    fread(&reg->tamNomeLinha, sizeof(int), 1, fileDados);
    //lê o nomeLinha, se não for um campo NULO
    if(reg->tamNomeLinha != 0){
        char *nomeLinha = (char*) malloc((sizeof(char) * reg->tamNomeLinha) + 1); // +1 para o caractere nulo
        if (!nomeLinha) {
            if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
            return -1;
        }
        fread(nomeLinha, sizeof(char), reg->tamNomeLinha, fileDados);
        nomeLinha[reg->tamNomeLinha] = '\0';
        reg->nomeLinha = nomeLinha;
    } else {
        //se for NULO, só indica que é
        reg->nomeLinha = "";
    }
    if (porCampo[CAMPO_NOME_LINHA] && verificarMatchStr(0, porCampo[CAMPO_NOME_LINHA]->valor, reg->nomeLinha)) numMatches++;

    return numMatches;
}


bool encontrarChave(No *no, int chave, int *subArvore, int *ponteiroDados){
    //pra fins de ordenação, as chaves que não existem tem peso infinito
    int C1 = no->C[0] != -1 ? no->C[0] : INT_MAX;
    int C2 = no->C[1] != -1 ? no->C[1] : INT_MAX;
    int C3 = no->C[2] != -1 ? no->C[2] : INT_MAX;

    //encontra ou a chave ou a subárvore em que ela deve se encontrar
    if(chave == C2){
        *ponteiroDados = no->Pr[1];
        return true;
    }
    else if(chave < C2){
        if(chave == C1){
            *ponteiroDados = no->Pr[0];
            return true;
        } else if(chave < C1) {
            *subArvore = no->P[0];
        }
        else {
            *subArvore = no->P[1];
        }
    }
    else {
        if(chave == C3){
            *ponteiroDados = no->Pr[2];
        return true;
    }
        else if(chave < C3){
            *subArvore = no->P[2];
        }
        else {
            *subArvore = no->P[3];
        }
    }
    return false;
}

void selectAll(FILE *file){
    fseek(file, TAM_CABECALHO, SEEK_SET); 

    Registro *reg = (Registro*) malloc(sizeof(Registro));
    if(!reg){
        fclose(file);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int regLidos = 0;
    char removido;
    while(fread(&removido, sizeof(char), 1, file)){
        if(removido == '1') {
            fseek(file, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            continue;
        }

        // garante estado limpo por iteração (evita usar ponteiros antigos quando o campo é nulo)
        reg->tamNomeEstacao = 0; reg->nomeEstacao = "";
        reg->tamNomeLinha = 0; reg->nomeLinha   = "";

        fseek(file, DADOS_OFF_PROXRRN - 1, SEEK_CUR); //pula os 4 bytes de proxRRN

        //lê os campos do registro e armazena na struct
        fread(&reg->codEstacao, sizeof(int), 1, file);
        fread(&reg->codLinha, sizeof(int), 1, file);
        fread(&reg->codProxEstacao, sizeof(int), 1, file);
        fread(&reg->distProxEstacao, sizeof(int), 1, file);
        fread(&reg->codLinhaIntegra, sizeof(int), 1, file);
        fread(&reg->codEstIntegra, sizeof(int), 1, file);

        fread(&reg->tamNomeEstacao, sizeof(int), 1, file);
        if(reg->tamNomeEstacao != 0){
            char *nomeEstacao = (char*) malloc(( sizeof(char) * reg->tamNomeEstacao ) + 1); // +1 para o caractere nulo
            if (!nomeEstacao) break;
            fread(nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
            nomeEstacao[reg->tamNomeEstacao] = '\0';
            reg->nomeEstacao = nomeEstacao;
        }

        fread(&reg->tamNomeLinha, sizeof(int), 1, file);
        if(reg->tamNomeLinha != 0){
            char *nomeLinha = (char*) malloc((sizeof(char) * reg->tamNomeLinha ) + 1); // +1 para o caractere nulo
            if (!nomeLinha) { 
                if (reg->tamNomeEstacao > 0) 
                    free(reg->nomeEstacao); 
                break; 
            }

            fread(nomeLinha, sizeof(char), reg->tamNomeLinha, file);
            nomeLinha[reg->tamNomeLinha] = '\0';
            reg->nomeLinha = nomeLinha;
        }
        regLidos++;

        printReg(reg);
        int tamRestante = TAM_LIVRE_REG(reg->tamNomeEstacao, reg->tamNomeLinha);
        if(tamRestante != 0) fseek(file, tamRestante, SEEK_CUR); //pula os $

        if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
        if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
    }

    if(regLidos == 0) printf("Registro inexistente.\n");
    free(reg);
    fclose(file);
}

int selectAllWhere(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares){
    int res = selectWhere(fileDados, fileIndice, pares, mPares, 0, false, true);
    return res;
}

int selectWhere(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares, int rrnInicial, bool apenasPrimeiroRes, bool seek){
    if(!fileDados) return -1;

    //se seek == true, vai para o rrn de início da busca
    //se rrnInicial == 0, precisa pular o cabeçalho
    //caso contrário, assume que o ponteiro do arquivo já está no lugar certo
    if(seek || rrnInicial == 0){
        long byteOffsetInicial = (long)TAM_CABECALHO + (long)rrnInicial * (long)TAM_REG;
        fseek(fileDados, byteOffsetInicial, SEEK_SET);
    }

    // indexa os pares por campo (posição fixa), posições sem filtro ficam como NULL
    CampoValor *porCampo[NUM_CAMPOS_REGISTRO];
    int numFiltros = popularParesPorCampo(pares, mPares, porCampo);

    if(porCampo[CAMPO_COD_ESTACAO] && fileIndice != NULL){
        return selectWhereIndexado(fileDados, fileIndice, porCampo, numFiltros, true);
    }

    Registro *reg = (Registro*) malloc(sizeof(Registro));
    if(!reg) return -1;

    int rrnAtual = rrnInicial;
    int numMatches = 0;

    bool encontrou = false;
    char removido;
    while(fread(&removido, sizeof(char), 1, fileDados)){
        if(removido == '1') {
            fseek(fileDados, TAM_REG-1, SEEK_CUR); //-1 porque, caso contrário, iria para o primeiro byte do codEstacao
            rrnAtual++;
            continue;
        }

        bool codEstacaoMatch;
        numMatches = confereCriteriosBusca(fileDados, reg, porCampo, &codEstacaoMatch);

        if(numMatches == numFiltros) {
            encontrou = true;
            if(apenasPrimeiroRes){
                if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
                if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
                free(reg);
                return rrnAtual;
            } else {
                printReg(reg);
                //se encontrou a estação pelo codEstacao, para a busca
                if(codEstacaoMatch){
                    if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
                    if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
                    break;
                }
            }
        }
        int tamRestante = TAM_LIVRE_REG(reg->tamNomeEstacao, reg->tamNomeLinha);
        if(tamRestante != 0) fseek(fileDados, tamRestante, SEEK_CUR); //pula os $

        if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
        if (reg->tamNomeLinha > 0) free(reg->nomeLinha);

        numMatches = 0;
        rrnAtual++;
    }

    //só imprime na tela se buscou por todos os resultados
    if(!apenasPrimeiroRes && !encontrou) printf("Registro inexistente.\n");
    if(!apenasPrimeiroRes) printf("\n");
    free(reg);

    if(!encontrou) return -1;
    else return 0;
}

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros, bool print){
    int chave = atoi(pares[CAMPO_COD_ESTACAO]->valor);
    //inicia a busca pelo nó raiz
    int rrnRaiz;
    fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    int rrnRes;
    int ponteiroRes;
    bool encontrou = buscaRecursiva(fileIndice, chave, rrnRaiz, &rrnRes, &ponteiroRes);

    if(!encontrou){
        if(print){
            printf("Registro inexistente.\n");
            printf("\n");
        }
        return -1;
    }
    else {
        fseek(fileDados, ponteiroRes, SEEK_SET);

        char removido;
        fread(&removido, sizeof(char), 1, fileDados);
        if(removido == '1'){
            if(print){
                printf("Registro inexistente.\n");
                printf("\n");
            }
            return -1;
        }

        pares[CAMPO_COD_ESTACAO] = NULL; //retira codEstacao dos criterios de busca
        Registro *reg = (Registro*) malloc(sizeof(Registro));
        bool dummy; //não vai ser utilizado
        //confere se o registro encontrado cumpre o restante dos critérios de busca
        int numMatches = confereCriteriosBusca(fileDados, reg, pares, &dummy);

        //numFiltros-1 porque o codEstacao foi retirado dos critérios de busca
        if(numMatches != numFiltros-1){
            if(print){
                printf("Registro inexistente.\n");
                printf("\n");
            }
            if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
            if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
            free(reg);
            return -1;
        }

        if(print){
            printReg(reg);
            printf("\n");
        }
        
        if (reg->tamNomeEstacao > 0) free(reg->nomeEstacao);
        if (reg->tamNomeLinha > 0) free(reg->nomeLinha);
        free(reg);
        return ponteiroRes;
    }
}

bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados){
    //final da recursão
    if(rrnNoAtual == -1) return false;

    int inicioNo = BTREE_NO_INICIO(rrnNoAtual);
    fseek(fileIndice, inicioNo, SEEK_SET);

    No *no = inicializarNo();
    lerNo(fileIndice, no);

    int subArvore;
    bool encontrou = encontrarChave(no, chave, &subArvore, ponteiroDados);
    if(!encontrou){
        bool res = buscaRecursiva(fileIndice, chave, subArvore, rrnNoRes, ponteiroDados);
        free(no);
        return res;
    }

    *rrnNoRes = rrnNoAtual;
    free(no);
    return true;
}