#include "../../headers/indice/btree_operacoes.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/indice/btree_no.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/dados/operacoes.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_CHAVES 1 // Em uma Árvore de ordem 4, mínimo é ceil(4/2) - 1 = 1
#define SUCESSO 1
#define CHAVE_NAO_ENCONTRADA 0
#define UNDERFLOW_PENDENTE 2

static bool lerCabecalhoIndice(FILE *fileIndice, int *noRaiz, int *topo, int *proxRRN, int *nroNos) {
    if (!fileIndice) return false;

    if (fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET) != 0) return false;
    if (fread(noRaiz, sizeof(int), 1, fileIndice) != 1) return false;
    if (fread(topo, sizeof(int), 1, fileIndice) != 1) return false;
    if (fread(proxRRN, sizeof(int), 1, fileIndice) != 1) return false;
    if (fread(nroNos, sizeof(int), 1, fileIndice) != 1) return false;

    return true;
}

static bool escreverCabecalhoIndice(FILE *fileIndice, char status, int noRaiz, int topo, int proxRRN, int nroNos) {
    if (!fileIndice) return false;

    if (fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET) != 0) return false;
    if (fwrite(&status, sizeof(char), 1, fileIndice) != 1) return false;
    if (fwrite(&noRaiz, sizeof(int), 1, fileIndice) != 1) return false;
    if (fwrite(&topo, sizeof(int), 1, fileIndice) != 1) return false;
    if (fwrite(&proxRRN, sizeof(int), 1, fileIndice) != 1) return false;
    if (fwrite(&nroNos, sizeof(int), 1, fileIndice) != 1) return false;

    return true;
}

bool encontrarPos(No *no, int chave, int *pos, int *ponteiroDados){
    int i = 0;

    if (!no) return false;

    // busca sequencial dentro do nó, já que o número máximo de chaves é pequeno (3)
    while (i < no->nroChaves && no->C[i] != -1 && chave > no->C[i]) i++;

    if (pos) *pos = i;

    if (i < no->nroChaves && no->C[i] == chave) {
        if (ponteiroDados) *ponteiroDados = no->Pr[i];
        return true;
    }

    return false;
}

static bool escreverNoNoRRN(FILE *fileIndice, int rrn, No *no) {
    long inicioNo = TAM_BTREE_CABECALHO + (long)rrn * (long)TAM_NO;

    if (!fileIndice || !no) return false;
    if (fseek(fileIndice, inicioNo, SEEK_SET) != 0) return false;

    return escreverNo(fileIndice, no);
}

static bool atualizarNoRaizIndice(FILE *fileIndice, int noRaiz) {
    if (!fileIndice) return false;

    if (fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET) != 0) return false;
    if (fwrite(&noRaiz, sizeof(int), 1, fileIndice) != 1) return false;

    return true;
}

static bool atualizarStatusIndice(FILE *fileIndice, char status) {
    if (!fileIndice) return false;

    if (fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET) != 0) return false;
    if (fwrite(&status, sizeof(char), 1, fileIndice) != 1) return false;

    return true;
}

static bool criarRaizInicial(FILE *fileIndice, int chave, int ponteiroDados, int *rrnRaiz) {
    No *raiz = criarNo(fileIndice, rrnRaiz);

    if (!raiz) return false;

    raiz->tipoNo = NO_FOLHA;
    raiz->nroChaves = 1;
    raiz->C[0] = chave;
    raiz->Pr[0] = ponteiroDados;
    raiz->P[0] = -1;
    raiz->P[1] = -1;

    if (!escreverNoNoRRN(fileIndice, *rrnRaiz, raiz)) {
        free(raiz);
        return false;
    }

    if (!atualizarNoRaizIndice(fileIndice, *rrnRaiz)) {
        free(raiz);
        return false;
    }

    free(raiz);
    return true;
}

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros){ // ver se muda ou não
    int chave = atoi(pares[CAMPO_COD_ESTACAO]->valor);
    int rrnRaiz;
    fseek(fileIndice, 1, SEEK_SET);
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    int rrnRes;
    int ponteiroRes;
    bool encontrou = buscaRecursiva(fileIndice, chave, rrnRaiz, &rrnRes, &ponteiroRes);

    if(!encontrou) return -1;
    else {
        fseek(fileDados, ponteiroRes, SEEK_SET);

        char removido;
        fread(&removido, sizeof(char), 1, fileDados);
        if(removido == '1'){
            printf("Registro inexistente.\n");
            return -1;
        }

        pares[CAMPO_COD_ESTACAO] = NULL; //retira codEstacao dos criterios de busca
        Registro *reg = (Registro*) malloc(sizeof(Registro));
        int numMatches = confereCriteriosBusca(fileDados, reg, pares);

        //numFiltros-1 porque o codEstacao foi retirado dos critérios de busca
        if(numMatches != numFiltros-1){
            printf("Registro inexistente.\n");
            return -1;
        }

        printReg(reg);
        return ponteiroRes;
    }
}

bool buscaRecursiva(FILE *fileIndice, int chave, int rrnNoAtual, int *rrnNoRes, int *ponteiroDados){
    if(rrnNoAtual == -1) return false;

    int inicioNo = TAM_BTREE_CABECALHO + rrnNoAtual * TAM_NO;
    fseek(fileIndice, inicioNo, SEEK_SET);

    No *no = incializarNo();
    lerNo(fileIndice, no);

    int subArvore;
    bool encontrou = encontrarChave(no, chave, &subArvore, ponteiroDados);

    if(!encontrou) return buscaRecursiva(fileIndice, chave, subArvore, rrnNoRes, ponteiroDados);

    *rrnNoRes = rrnNoAtual;
    return true;
}

static No *split(FILE *fileIndice, No *no, int posicaoInsercao, int chaveNova, int ponteiroDadosChaveNova, int filhoDirChaveNova, int *chaveASerPromovida, int *ponteiroDadosPromovido, int *filhoDirChaveASerPromovida) {
    int rrnNovoNo;
    No *novoNo = criarNo(fileIndice, &rrnNovoNo);

    if (!novoNo) return NULL;

    *chaveASerPromovida = distribuirOrdenado(no, novoNo, posicaoInsercao, chaveNova, ponteiroDadosChaveNova, no->P[posicaoInsercao], filhoDirChaveNova, ponteiroDadosPromovido);
    *filhoDirChaveASerPromovida = rrnNovoNo;

    return novoNo;
}

static bool criarNovaRaiz(FILE *fileIndice, int rrnRaizAtual, int chavePromocao, int ponteiroDadosPromocao, int filhoDirPromocao){
    int rrnNovaRaiz;
    No *novaRaiz = criarNo(fileIndice, &rrnNovaRaiz);

    if(!novaRaiz) return false;

    novaRaiz->tipoNo = NO_RAIZ;
    novaRaiz->nroChaves = 1;
    novaRaiz->C[0] = chavePromocao;
    novaRaiz->Pr[0] = ponteiroDadosPromocao;
    novaRaiz->P[0] = rrnRaizAtual; // filho da esquerda é a raiz antiga
    novaRaiz->P[1] = filhoDirPromocao; // filho da direita é o nó criado no split

    if(!escreverNoNoRRN(fileIndice, rrnNovaRaiz, novaRaiz)) {
        free(novaRaiz);
        return false;
    }

    if(!atualizarNoRaizIndice(fileIndice, rrnNovaRaiz)) {
        free(novaRaiz);
        return false;
    }

    free(novaRaiz);
    return true;
}

int insertIndice(FILE *fileIndice, int chave, int ponteiroDados){
    int rrnRaiz, topo, proxRRN, nroNos;
    int chavePromocao, ponteiroDadosPromocao, filhoDirPromocao;
    int res;

    if(!fileIndice || !lerCabecalhoIndice(fileIndice, &rrnRaiz, &topo, &proxRRN, &nroNos)) return ERRO_DE_INSERCAO;

    if(rrnRaiz == -1){
        return criarRaizInicial(fileIndice, chave, ponteiroDados, &rrnRaiz) ? 1 : ERRO_DE_INSERCAO;
    }

    res = insertIndiceRec(fileIndice, chave, ponteiroDados, rrnRaiz, &chavePromocao, &ponteiroDadosPromocao, &filhoDirPromocao);
    if(res == PROMOCAO){
        if(!criarNovaRaiz(fileIndice, rrnRaiz, chavePromocao, ponteiroDadosPromocao, filhoDirPromocao)) return ERRO_DE_INSERCAO;
    }

    return res == ERRO_DE_INSERCAO ? ERRO_DE_INSERCAO : 1;
}

int insertIndiceRec(FILE *fileIndice, int chave, int ponteiroDados, int rrnNoAtual, int *chaveASerPromovida, int *ponteiroDadosPromovido, int *filhoDirChaveASerPromovida){
    int inicioNo;
    No *no;
    int posicao, ponteiroEncontrado, subArvore;
    int chavePromovidaFilho, ponteiroDadosPromovidoFilho, filhoDirPromovido;

    if(rrnNoAtual == -1){
        *chaveASerPromovida = chave;
        *ponteiroDadosPromovido = ponteiroDados;
        *filhoDirChaveASerPromovida = -1;
        return PROMOCAO;
    }

    inicioNo = TAM_BTREE_CABECALHO + rrnNoAtual * TAM_NO;
    fseek(fileIndice, inicioNo, SEEK_SET);

    no = incializarNo();
    if(!no) return ERRO_DE_INSERCAO;

    lerNo(fileIndice, no);

    if(encontrarPos(no, chave, &posicao, &ponteiroEncontrado)) {
        free(no);
        return ERRO_DE_INSERCAO;
    }

    subArvore = no->P[posicao];
    free(no);

    int res = insertIndiceRec(fileIndice, chave, ponteiroDados, subArvore, &chavePromovidaFilho, &ponteiroDadosPromovidoFilho, &filhoDirPromovido);

    if(res == SEM_PROMOCAO || res == ERRO_DE_INSERCAO) return res;

    no = incializarNo();
    if(!no) return ERRO_DE_INSERCAO;

    fseek(fileIndice, inicioNo, SEEK_SET);
    lerNo(fileIndice, no);

    if (no->nroChaves < NRO_MAX_CHAVES) {
        insereOrdenado(no, posicao, chavePromovidaFilho, ponteiroDadosPromovidoFilho, subArvore, filhoDirPromovido);

        if(!escreverNoNoRRN(fileIndice, rrnNoAtual, no)) {
            free(no);
            return ERRO_DE_INSERCAO;
        }

        free(no);
        return SEM_PROMOCAO;
    }

    No *novoNo = split(fileIndice, no, posicao, chavePromovidaFilho, ponteiroDadosPromovidoFilho, filhoDirPromovido, chaveASerPromovida, ponteiroDadosPromovido, filhoDirChaveASerPromovida);
    if(!novoNo) {
        free(no);
        return ERRO_DE_INSERCAO;
    }

    if(!escreverNoNoRRN(fileIndice, rrnNoAtual, no) || !escreverNoNoRRN(fileIndice, *filhoDirChaveASerPromovida, novoNo)) {
        free(no);
        free(novoNo);
        return ERRO_DE_INSERCAO;
    }

    free(no);
    free(novoNo);
    return PROMOCAO;
}

bool criarIndiceArvoreB(char *arquivoDados, char *arquivoIndice){
    FILE *fileDados = NULL;
    FILE *fileIndice = NULL;
    char statusDados;
    bool ok = true;
    long offsetRegistro;

    if(!arquivoDados || !arquivoIndice) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    fileDados = fopen(arquivoDados, "rb");
    fileIndice = fopen(arquivoIndice, "wb+");

    if(!fileDados || !fileIndice) {
        printf("Falha no processamento do arquivo.\n");
        if(fileDados) fclose(fileDados);
        if(fileIndice) fclose(fileIndice);
        return false;
    }

    if(fread(&statusDados, sizeof(char), 1, fileDados) != 1 || statusDados != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados);
        fclose(fileIndice);
        return false;
    }

    if(!escreverCabecalhoIndice(fileIndice, '0', -1, -1, 0, 0)) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados);
        fclose(fileIndice);
        return false;
    }

    if(fseek(fileDados, TAM_CABECALHO, SEEK_SET) != 0) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados);
        fclose(fileIndice);
        return false;
    }

    offsetRegistro = TAM_CABECALHO;

    while (1) {
        char removido;
        int lixo, chave, tamNomeEstacao, tamNomeLinha;
        int i;

        if (fread(&removido, sizeof(char), 1, fileDados) != 1) break;

        if (removido == '1') {
            if (fseek(fileDados, TAM_REG - 1, SEEK_CUR) != 0) {
                ok = false;
                break;
            }
            offsetRegistro += TAM_REG;
            continue;
        }

        if (fread(&lixo, sizeof(int), 1, fileDados) != 1 || fread(&chave, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }

        for (i = 0; i < 5; i++) {
            if (fread(&lixo, sizeof(int), 1, fileDados) != 1) {
                ok = false;
                break;
            }
        }
        if (!ok) break;

        if (fread(&tamNomeEstacao, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }
        if (tamNomeEstacao > 0 && fseek(fileDados, tamNomeEstacao, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        if (fread(&tamNomeLinha, sizeof(int), 1, fileDados) != 1) {
            ok = false;
            break;
        }
        if (tamNomeLinha > 0 && fseek(fileDados, tamNomeLinha, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        i = TAM_LIVRE_REG(tamNomeEstacao, tamNomeLinha);
        if (i > 0 && fseek(fileDados, i, SEEK_CUR) != 0) {
            ok = false;
            break;
        }

        // printf("Inserindo chave=%d offset=%ld\n", chave, offsetRegistro); // debug puramente
        if (insertIndice(fileIndice, chave, (int)offsetRegistro) == ERRO_DE_INSERCAO) {
            ok = false;
            break;
        }

        offsetRegistro += TAM_REG;
    }

    if (ok) {
        ok = atualizarStatusIndice(fileIndice, '1');
    }

    fclose(fileDados);
    fclose(fileIndice);

    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    return true;
}

// Função driver que inicia a cadeia de remoção e lida com reduções de altura na raiz
bool removerChaveIndice(FILE *fileIndice, int chave) {
    int rrnRaiz;
    fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    if (rrnRaiz == -1) return false;

    int status = removerRecursivo(fileIndice, rrnRaiz, chave);
    
    if (status == CHAVE_NAO_ENCONTRADA) {
        return false; 
    }

    /*
    // nroNos decrementa a cada sucesso de remoção, mesmo que seja apenas lógica
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    int nroNos;
    fread(&nroNos, sizeof(int), 1, fileIndice);
    // nroNos--;
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    fwrite(&nroNos, sizeof(int), 1, fileIndice);
    */

    // esvaziamento de raiz tratado somente após a remoção recursiva para evitar casos de underflow pendente na raiz
    No *raiz = incializarNo();
    fseek(fileIndice, TAM_BTREE_CABECALHO + rrnRaiz * TAM_NO, SEEK_SET);
    lerNo(fileIndice, raiz);

    if (raiz->nroChaves == 0) {
        // Usa a verificação de ponteiros para saber se é folha
        if (raiz->P[0] != -1) {
            int novaRaiz = raiz->P[0]; 
            
            fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
            fwrite(&novaRaiz, sizeof(int), 1, fileIndice);

            apagarNo(fileIndice, rrnRaiz);
            
            No *nRaiz = incializarNo();
            fseek(fileIndice, TAM_BTREE_CABECALHO + novaRaiz * TAM_NO, SEEK_SET);
            lerNo(fileIndice, nRaiz);
            
            // Força a nova raiz a ter a flag 0, independentemente de ser folha ou não
            nRaiz->tipoNo = NO_RAIZ;
            
            fseek(fileIndice, TAM_BTREE_CABECALHO + novaRaiz * TAM_NO, SEEK_SET);
            escreverNo(fileIndice, nRaiz);
            free(nRaiz);

        } else {
            int vazio = -1;
            fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
            fwrite(&vazio, sizeof(int), 1, fileIndice);
            apagarNo(fileIndice, rrnRaiz);
        }
    }
    free(raiz);
    return true; 
}

// Remoção lógica recursiva descendente e propagação de underflow ascendente
int removerRecursivo(FILE *fileIndice, int rrnAtual, int chave) {
    if (rrnAtual == -1) return CHAVE_NAO_ENCONTRADA;

    No *noAtual = incializarNo();
    fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET);
    lerNo(fileIndice, noAtual);

    int pos = 0;
    while (pos < noAtual->nroChaves && noAtual->C[pos] < chave) pos++;

    bool encontrada = (pos < noAtual->nroChaves && noAtual->C[pos] == chave);

    if (encontrada) {
        if (noAtual->P[0] == -1) {
            // Remoção física do registro na folha via shift
            for (int i = pos; i < noAtual->nroChaves - 1; i++) {
                noAtual->C[i] = noAtual->C[i+1];
                noAtual->Pr[i] = noAtual->Pr[i+1];
            }
            noAtual->nroChaves--;
            noAtual->C[noAtual->nroChaves] = -1;
            noAtual->Pr[noAtual->nroChaves] = -1;            
        } else {
            // Substituição pela chave sucessora em nós internos
            int rrnSucessor = noAtual->P[pos+1];
            No *sucessor = incializarNo();
            fseek(fileIndice, TAM_BTREE_CABECALHO + rrnSucessor * TAM_NO, SEEK_SET);
            lerNo(fileIndice, sucessor);
            
            while (sucessor->P[0] != -1) {
                rrnSucessor = sucessor->P[0];
                fseek(fileIndice, TAM_BTREE_CABECALHO + rrnSucessor * TAM_NO, SEEK_SET);
                lerNo(fileIndice, sucessor);
            }
            
            int chaveSucessora = sucessor->C[0];
            int ponteiroDadosSucessor = sucessor->Pr[0];
            free(sucessor);

            noAtual->C[pos] = chaveSucessora;
            noAtual->Pr[pos] = ponteiroDadosSucessor;
            fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET);
            escreverNo(fileIndice, noAtual);

            // Chamada recursiva para apagar a sucessora promovida
            int statusSub = removerRecursivo(fileIndice, noAtual->P[pos+1], chaveSucessora);
            if (statusSub == UNDERFLOW_PENDENTE) {
                tratarUnderflow(fileIndice, noAtual, rrnAtual, pos + 1);
            }
        }
    } else {
        if (noAtual->P[0] == -1) {
            free(noAtual);
            return CHAVE_NAO_ENCONTRADA; 
        }

        // Continua a busca na subárvore adequada
        int statusSub = removerRecursivo(fileIndice, noAtual->P[pos], chave);
        if (statusSub == CHAVE_NAO_ENCONTRADA) {
            free(noAtual);
            return CHAVE_NAO_ENCONTRADA;
        }

        if (statusSub == UNDERFLOW_PENDENTE) {
            tratarUnderflow(fileIndice, noAtual, rrnAtual, pos);
        }
    }

    fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET);
    escreverNo(fileIndice, noAtual);

    int chavesAtuais = noAtual->nroChaves;
    free(noAtual);

    if (chavesAtuais < MIN_CHAVES) {
        return UNDERFLOW_PENDENTE;
    }

    return SUCESSO;
}

// Analisa os irmãos adjacentes para executar empréstimo (redistribuição) ou fusão (merge)
void tratarUnderflow(FILE *fileIndice, No *pai, int rrnPai, int indicePonteiroFilho) {
    int rrnAtual = pai->P[indicePonteiroFilho];
    No *atual = incializarNo(); fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET); lerNo(fileIndice, atual);
    
    No *irmEsq = NULL; int rrnIrmEsq = -1;
    if (indicePonteiroFilho > 0) {
        rrnIrmEsq = pai->P[indicePonteiroFilho - 1]; irmEsq = incializarNo(); fseek(fileIndice, TAM_BTREE_CABECALHO + rrnIrmEsq * TAM_NO, SEEK_SET); lerNo(fileIndice, irmEsq);
    }

    No *irmDir = NULL; int rrnIrmDir = -1;
    if (indicePonteiroFilho < pai->nroChaves) {
        rrnIrmDir = pai->P[indicePonteiroFilho + 1]; irmDir = incializarNo(); fseek(fileIndice, TAM_BTREE_CABECALHO + rrnIrmDir * TAM_NO, SEEK_SET); lerNo(fileIndice, irmDir);
    }

    // 1. Empréstimo da DIREITA
    if (irmDir != NULL && irmDir->nroChaves > MIN_CHAVES) {
        int chavesTotal = 1 + irmDir->nroChaves;
        int chavesEsq = chavesTotal / 2; // Garante a regra: "nó mais à esquerda deverá conter uma chave a mais"
        int chavesDir = chavesTotal - chavesEsq - 1;

        int bufC[4], bufPr[4], bufP[5];
        bufC[0] = pai->C[indicePonteiroFilho]; bufPr[0] = pai->Pr[indicePonteiroFilho]; bufP[0] = atual->P[0];
        for(int i = 0; i < irmDir->nroChaves; i++) {
            bufC[i+1] = irmDir->C[i]; bufPr[i+1] = irmDir->Pr[i]; bufP[i+1] = irmDir->P[i];
        }
        bufP[irmDir->nroChaves + 1] = irmDir->P[irmDir->nroChaves];

        atual->nroChaves = chavesEsq;
        for(int i = 0; i < chavesEsq; i++) { atual->C[i] = bufC[i]; atual->Pr[i] = bufPr[i]; atual->P[i] = bufP[i]; }
        atual->P[chavesEsq] = bufP[chavesEsq];

        pai->C[indicePonteiroFilho] = bufC[chavesEsq]; pai->Pr[indicePonteiroFilho] = bufPr[chavesEsq];

        irmDir->nroChaves = chavesDir;
        for(int i = 0; i < chavesDir; i++) {
            irmDir->C[i] = bufC[chavesEsq + 1 + i]; irmDir->Pr[i] = bufPr[chavesEsq + 1 + i]; irmDir->P[i] = bufP[chavesEsq + 1 + i];
        }
        irmDir->P[chavesDir] = bufP[chavesEsq + 1 + chavesDir];
        for(int i = chavesDir; i < 3; i++) { irmDir->C[i] = -1; irmDir->Pr[i] = -1; irmDir->P[i+1] = -1; }

        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnPai * TAM_NO, SEEK_SET); escreverNo(fileIndice, pai);
        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET); escreverNo(fileIndice, atual);
        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnIrmDir * TAM_NO, SEEK_SET); escreverNo(fileIndice, irmDir);

        free(atual); if(irmEsq) free(irmEsq); free(irmDir); return;
    }

    // 2. Empréstimo da ESQUERDA
    if (irmEsq != NULL && irmEsq->nroChaves > MIN_CHAVES) {
        int chavesTotal = irmEsq->nroChaves + 1;
        int chavesEsq = chavesTotal / 2;
        int chavesDir = chavesTotal - chavesEsq - 1;

        int bufC[4], bufPr[4], bufP[5];
        for(int i = 0; i < irmEsq->nroChaves; i++) {
            bufC[i] = irmEsq->C[i]; bufPr[i] = irmEsq->Pr[i]; bufP[i] = irmEsq->P[i];
        }
        bufP[irmEsq->nroChaves] = irmEsq->P[irmEsq->nroChaves];
        bufC[irmEsq->nroChaves] = pai->C[indicePonteiroFilho - 1]; bufPr[irmEsq->nroChaves] = pai->Pr[indicePonteiroFilho - 1];
        bufP[irmEsq->nroChaves + 1] = atual->P[0];

        irmEsq->nroChaves = chavesEsq;
        for(int i = 0; i < chavesEsq; i++) { irmEsq->C[i] = bufC[i]; irmEsq->Pr[i] = bufPr[i]; irmEsq->P[i] = bufP[i]; }
        irmEsq->P[chavesEsq] = bufP[chavesEsq];
        for(int i = chavesEsq; i < 3; i++) { irmEsq->C[i] = -1; irmEsq->Pr[i] = -1; irmEsq->P[i+1] = -1; }

        pai->C[indicePonteiroFilho - 1] = bufC[chavesEsq]; pai->Pr[indicePonteiroFilho - 1] = bufPr[chavesEsq];

        atual->nroChaves = chavesDir;
        for(int i = 0; i < chavesDir; i++) {
            atual->C[i] = bufC[chavesEsq + 1 + i]; atual->Pr[i] = bufPr[chavesEsq + 1 + i]; atual->P[i] = bufP[chavesEsq + 1 + i];
        }
        atual->P[chavesDir] = bufP[chavesEsq + 1 + chavesDir];

        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnPai * TAM_NO, SEEK_SET); escreverNo(fileIndice, pai);
        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET); escreverNo(fileIndice, atual);
        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnIrmEsq * TAM_NO, SEEK_SET); escreverNo(fileIndice, irmEsq);

        free(atual); free(irmEsq); if(irmDir) free(irmDir); return;
    }

    // 3. MERGE ESQUERDA (O Enunciado exige tentar Esquerda primeiro na Concatenação)
    if (irmEsq != NULL) {
        fazerMerge(fileIndice, irmEsq, atual, pai, rrnIrmEsq, rrnAtual, rrnPai, indicePonteiroFilho - 1);
    }
    // 4. MERGE DIREITA (Se não for possível esquerda)
    else if (irmDir != NULL) {
        fazerMerge(fileIndice, atual, irmDir, pai, rrnAtual, rrnIrmDir, rrnPai, indicePonteiroFilho);
    }
    
    free(atual); if(irmEsq) free(irmEsq); if(irmDir) free(irmDir);
}

// Une fisicamente duas páginas e rebaixa a chave separadora do pai
void fazerMerge(FILE *fileIndice, No *esq, No *dir, No *pai, int rrnEsq, int rrnDir, int rrnPai, int indiceChavePai) {
    esq->C[esq->nroChaves] = pai->C[indiceChavePai];
    esq->Pr[esq->nroChaves] = pai->Pr[indiceChavePai];
    esq->nroChaves++;

    for (int i = 0; i < dir->nroChaves; i++) {
        esq->C[esq->nroChaves + i] = dir->C[i];
        esq->Pr[esq->nroChaves + i] = dir->Pr[i];
        esq->P[esq->nroChaves + i] = dir->P[i];
    }
    esq->P[esq->nroChaves + dir->nroChaves] = dir->P[dir->nroChaves];
    esq->nroChaves += dir->nroChaves;

    for (int i = indiceChavePai; i < pai->nroChaves - 1; i++) {
        pai->C[i] = pai->C[i+1];
        pai->Pr[i] = pai->Pr[i+1];
        pai->P[i+1] = pai->P[i+2];
    }
    pai->nroChaves--;
    pai->C[pai->nroChaves] = -1;
    pai->Pr[pai->nroChaves] = -1;
    pai->P[pai->nroChaves + 1] = -1;

    fseek(fileIndice, TAM_BTREE_CABECALHO + rrnEsq * TAM_NO, SEEK_SET); 
    escreverNo(fileIndice, esq);
    
    fseek(fileIndice, TAM_BTREE_CABECALHO + rrnPai * TAM_NO, SEEK_SET); 
    escreverNo(fileIndice, pai);
    
    apagarNo(fileIndice, rrnDir);
}

bool deleteWhereIndexado(char *arquivoEntrada, char *arquivoIndice, CampoValor *pares, int mPares) {
    FILE *fileDados = fopen(arquivoEntrada, "r+b");
    FILE *fileIndice = fopen(arquivoIndice, "r+b");
    if (!fileDados || !fileIndice) {
        if (fileDados) fclose(fileDados);
        if (fileIndice) fclose(fileIndice);
        return false;
    }

    char statusDados, statusIndice;
    int topoDados;

    if (fread(&statusDados, sizeof(char), 1, fileDados) != 1 || statusDados != '1' || fread(&topoDados, sizeof(int), 1, fileDados) != 1) {
        fclose(fileDados); fclose(fileIndice); return false;
    }
    if (fread(&statusIndice, sizeof(char), 1, fileIndice) != 1 || statusIndice != '1') {
        fclose(fileDados); fclose(fileIndice); return false;
    }

    atualizarStatus(fileDados, '0', true);
    atualizarStatusIndice(fileIndice, '0');

    bool ok = true;
    int idxCodEstacao = encontrarIndexCampo(pares, mPares, "codEstacao");

    if (idxCodEstacao != -1 && !valorEhNulo(pares[idxCodEstacao].valor)) {
            int chaveBuscada = atoi(pares[idxCodEstacao].valor);
            int rrnRaiz; fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET); fread(&rrnRaiz, sizeof(int), 1, fileIndice);

            int rrnResIndice, ponteiroResDados;
            if (buscaRecursiva(fileIndice, chaveBuscada, rrnRaiz, &rrnResIndice, &ponteiroResDados)) {
                fseek(fileDados, ponteiroResDados, SEEK_SET);
                char removido; fread(&removido, sizeof(char), 1, fileDados);
                
                if (removido == '0') {
                    // --- CORREÇÃO 1: FILTRAGEM COMPLETA ---
                    int rrnAlvo = (ponteiroResDados - TAM_CABECALHO) / TAM_REG;
                    
                    // Valida se os outros filtros (nome, linha, etc) também batem no registro encontrado
                    int rrnConfirmado = selectWhere(fileDados, NULL, pares, mPares, rrnAlvo, true, true);
                    
                    if (rrnConfirmado == rrnAlvo) { // Todos os filtros bateram! 
                        fseek(fileDados, ponteiroResDados, SEEK_SET); 
                        char flag = '1';
                        fwrite(&flag, sizeof(char), 1, fileDados); 
                        fwrite(&topoDados, sizeof(int), 1, fileDados);
                        
                        topoDados = rrnAlvo;
                        fseek(fileDados, 1, SEEK_SET); 
                        fwrite(&topoDados, sizeof(int), 1, fileDados);
                        
                        removerChaveIndice(fileIndice, chaveBuscada);
                    }
                    // Se não bateu os filtros, ignora. O registro não deve ser apagado!
                }
            }
        } else {
        // Cenário 2: Busca sequencial O(n) lendo diretamente no disco
        int rrn = -1;
        while (true) {
            // Passamos NULL para o índice aqui forçando o full-scan no selectWhere
            rrn = selectWhere(fileDados, NULL, pares, mPares, rrn + 1, true, true);
            if (rrn < 0) break;

            long inicioRegistro = (long)TAM_CABECALHO + (long)rrn * (long)TAM_REG;
            
            // Lê a chave (codEstacao) da estrutura do registro antes de marcar como removido
            // Pula: flag removido (1 byte) + proxRRN (4 bytes)
            fseek(fileDados, inicioRegistro + 5, SEEK_SET); 
            int chaveParaRemover;
            if (fread(&chaveParaRemover, sizeof(int), 1, fileDados) != 1) { ok = false; break; }

            // Marca o registro como removido
            fseek(fileDados, inicioRegistro, SEEK_SET);
            char flag = '1';
            if (fwrite(&flag, sizeof(char), 1, fileDados) != 1) { ok = false; break; }
            if (fwrite(&topoDados, sizeof(int), 1, fileDados) != 1) { ok = false; break; }
            
            topoDados = rrn;

            // Remove a chave resgatada da Árvore-B
            if (!removerChaveIndice(fileIndice, chaveParaRemover)) {
                ok = false; break;
            }
        }
        if (ok) {
            fseek(fileDados, 1, SEEK_SET);
            fwrite(&topoDados, sizeof(int), 1, fileDados);
        }
    }

    recalcularContadores(fileDados);

    atualizarStatus(fileDados, '1', true);
    atualizarStatusIndice(fileIndice, '1');

    fclose(fileDados);
    fclose(fileIndice);

    return ok;
}