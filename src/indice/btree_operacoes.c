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

static bool atualizarStatusIndice(FILE *fileIndice, char status) {
    if (!fileIndice) return false;

    if (fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET) != 0) return false;
    if (fwrite(&status, sizeof(char), 1, fileIndice) != 1) return false;

    return true;
}

int selectWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares[8], int numFiltros){
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
    if(!encontrou){
        bool res = buscaRecursiva(fileIndice, chave, subArvore, rrnNoRes, ponteiroDados);
        free(no);
        return res;
    }

    *rrnNoRes = rrnNoAtual;
    return true;
}

No *split(FILE *fileIndice, No *no, int chaveNova, int ponteiroDadosChaveNova, int filhoDirChaveNova, int *chaveASerPromovida, int *filhoDirChaveASerPromovida){
    int rrnNovoNo;
    No *novoNo = criarNo(fileIndice, &rrnNovoNo);
    int chaveMeio = distribuirOrdenado(fileIndice, no, novoNo, chaveNova, filhoDirChaveNova);
    *chaveASerPromovida = chaveMeio;
    *filhoDirChaveASerPromovida = rrnNovoNo;

    return novoNo;
}

void criarNovaRaiz(FILE *fileIndice, int rrnRaizAtual, int chavePromocao, int filhoDirPromocao){
    int rrnNovaRaiz;
    No *novaRaiz = criarNo(fileIndice, &rrnNovaRaiz);

    novaRaiz->nroChaves = 1;
    novaRaiz->C[0] = chavePromocao;
    novaRaiz->P[0] = rrnRaizAtual;          //filho a esquerda é a raiz antiga
    novaRaiz->P[1] = filhoDirPromocao; //filho a direita é o nó criado no split
    novaRaiz->tipoNo = NO_RAIZ;

    //salva a nova raiz no arquivo
    int inicioNovaRaiz = TAM_BTREE_CABECALHO + rrnNovaRaiz * TAM_NO;
    fseek(fileIndice, inicioNovaRaiz, SEEK_SET);
    escreverNo(fileIndice, novaRaiz);

    //atualiza o cabeçalho do arquivo para apontar para o RRN da nova raiz
    fseek(fileIndice, 1, SEEK_SET); 
    fwrite(&rrnNovaRaiz, sizeof(int), 1, fileIndice);
        
    free(novaRaiz);
}

int insertIndice(FILE *fileIndice, int chave, int ponteiroDados){
    int rrnRaiz;
    fseek(fileIndice, 1, SEEK_SET); //(verificar se é isso mesmo)
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    int chavePromocao;
    int filhoDirPromocao;
    int res = insertIndiceRec(fileIndice, chave, ponteiroDados, rrnRaiz, &chavePromocao, &filhoDirPromocao);
    if(res == PROMOCAO) criarNovaRaiz(fileIndice, rrnRaiz, chavePromocao, filhoDirPromocao);

    //atualiza o número de nós da árvore
    fseek(fileIndice, 13, SEEK_SET);
    int nroNos;
    fread(&nroNos, sizeof(int), 1, fileIndice);
    nroNos++;
    fseek(fileIndice, 13, SEEK_SET);
    fwrite(&nroNos, sizeof(int), 1, fileIndice);

    //atualiza o status de consistência
    fseek(fileIndice, 0, SEEK_SET);
    char consistente = '1';
    fwrite(&consistente, sizeof(char), 1, fileIndice);
    return 1;
}

int insertIndiceRec(FILE *fileIndice, int chave, int ponteiroDados, int rrnNoAtual, int *chaveASerPromovida, int *filhoDirChaveASerPromovida){
    if(rrnNoAtual == -1){
        *chaveASerPromovida = chave;
        *filhoDirChaveASerPromovida = -1;
        return PROMOCAO;
    }

    int inicioNo = TAM_BTREE_CABECALHO + rrnNoAtual * TAM_NO;
    fseek(fileIndice, inicioNo, SEEK_SET);

    No *no = incializarNo();
    lerNo(fileIndice, no);

    int subArvore;
    int dummy; //não vai ser utilizado
    bool encontrou = encontrarChave(no, chave, &subArvore, &dummy);
    if(encontrou) return ERRO_DE_INSERCAO;

    int chavePromovida;
    int filhoDirPromovido;
    int res = insertIndiceRec(fileIndice, chave, ponteiroDados, subArvore, &chavePromovida, &filhoDirPromovido);

    if(res == SEM_PROMOCAO || res == ERRO_DE_INSERCAO) return res;
    else if (no->nroChaves < NRO_MAX_CHAVES){
        insereOrdenado(no, chavePromovida, ponteiroDados, filhoDirPromovido);
        if(no->tipoNo != NO_RAIZ) no->tipoNo = NO_FOLHA; // Proteção para não sobrescrever raiz

        fseek(fileIndice, inicioNo, SEEK_SET);
        escreverNo(fileIndice, no);
        return SEM_PROMOCAO;
    } else {
        No *novoNo = split(fileIndice, no, chavePromovida, ponteiroDados, filhoDirPromovido, chaveASerPromovida, filhoDirChaveASerPromovida);

        fseek(fileIndice, inicioNo, SEEK_SET);
        escreverNo(fileIndice, no);
        if(no->tipoNo != NO_RAIZ) no->tipoNo = NO_INTERMEDIARIO;

        int inicioNovoNo = TAM_BTREE_CABECALHO + *filhoDirChaveASerPromovida * TAM_NO;
        fseek(fileIndice, inicioNovoNo, SEEK_SET);
        escreverNo(fileIndice, novoNo);
        return PROMOCAO;
    }

    return 0;
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

    // nroNos decrementa a cada sucesso de remoção, mesmo que seja apenas lógica
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    int nroNos;
    fread(&nroNos, sizeof(int), 1, fileIndice);
    nroNos--;
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    fwrite(&nroNos, sizeof(int), 1, fileIndice);

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
            
            No *nRaiz = incializarNo();
            fseek(fileIndice, TAM_BTREE_CABECALHO + novaRaiz * TAM_NO, SEEK_SET);
            lerNo(fileIndice, nRaiz);
            
            // Força a nova raiz a ter a flag 0, independentemente de ser folha ou não
            nRaiz->tipoNo = NO_RAIZ;
            
            fseek(fileIndice, TAM_BTREE_CABECALHO + novaRaiz * TAM_NO, SEEK_SET);
            escreverNo(fileIndice, nRaiz);
            free(nRaiz);

            apagarNo(fileIndice, rrnRaiz);
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
            noAtual->C[noAtual->nroChaves - 1] = -1;
            noAtual->Pr[noAtual->nroChaves - 1] = -1;
            noAtual->nroChaves--;
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
    No *atual = incializarNo();
    fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET);
    lerNo(fileIndice, atual);

    No *irmEsq = NULL;
    int rrnIrmEsq = -1;
    if (indicePonteiroFilho > 0) {
        rrnIrmEsq = pai->P[indicePonteiroFilho - 1];
        irmEsq = incializarNo();
        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnIrmEsq * TAM_NO, SEEK_SET);
        lerNo(fileIndice, irmEsq);
    }

    No *irmDir = NULL;
    int rrnIrmDir = -1;
    if (indicePonteiroFilho < pai->nroChaves) {
        rrnIrmDir = pai->P[indicePonteiroFilho + 1];
        irmDir = incializarNo();
        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnIrmDir * TAM_NO, SEEK_SET);
        lerNo(fileIndice, irmDir);
    }

    // redistribuição para o irmão direito somente, conforme especificação do trabalho 2
    if (irmDir != NULL && irmDir->nroChaves > MIN_CHAVES) {
        atual->C[atual->nroChaves] = pai->C[indicePonteiroFilho];
        atual->Pr[atual->nroChaves] = pai->Pr[indicePonteiroFilho];
        
        atual->P[atual->nroChaves + 1] = irmDir->P[0];
        
        pai->C[indicePonteiroFilho] = irmDir->C[0];
        pai->Pr[indicePonteiroFilho] = irmDir->Pr[0];
        
        for (int i = 0; i < irmDir->nroChaves - 1; i++) {
            irmDir->C[i] = irmDir->C[i+1];
            irmDir->Pr[i] = irmDir->Pr[i+1];
            irmDir->P[i] = irmDir->P[i+1];
        }
        irmDir->P[irmDir->nroChaves - 1] = irmDir->P[irmDir->nroChaves];
        
        irmDir->C[irmDir->nroChaves - 1] = -1;
        irmDir->Pr[irmDir->nroChaves - 1] = -1;
        irmDir->P[irmDir->nroChaves] = -1;
        
        atual->nroChaves++;
        irmDir->nroChaves--;

        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnPai * TAM_NO, SEEK_SET); escreverNo(fileIndice, pai);
        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET); escreverNo(fileIndice, atual);
        fseek(fileIndice, TAM_BTREE_CABECALHO + rrnIrmDir * TAM_NO, SEEK_SET); escreverNo(fileIndice, irmDir);
        
        free(atual); if(irmEsq) free(irmEsq); if(irmDir) free(irmDir);
        return;
    }

    // concatenacao primeiro com a direita, depois com a esquerda, conforme especificação do trabalho 2
    if (irmDir != NULL) {
        fazerMerge(fileIndice, atual, irmDir, pai, rrnAtual, rrnIrmDir, rrnPai, indicePonteiroFilho);
    } 
    // caso não seja possível, concatenacao à esquerda
    else if (irmEsq != NULL) {
        fazerMerge(fileIndice, irmEsq, atual, pai, rrnIrmEsq, rrnAtual, rrnPai, indicePonteiroFilho - 1);
    }
    
    free(atual); 
    if(irmEsq) free(irmEsq); 
    if(irmDir) free(irmDir);
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
    pai->C[pai->nroChaves - 1] = -1;
    pai->Pr[pai->nroChaves - 1] = -1;
    pai->P[pai->nroChaves] = -1;
    pai->nroChaves--;

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
        // Cenário 1: Busca otimizada O(log n) via Árvore-B
        int chaveBuscada = atoi(pares[idxCodEstacao].valor);
        int rrnRaiz;
        fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
        fread(&rrnRaiz, sizeof(int), 1, fileIndice);

        int rrnResIndice, ponteiroResDados;
        if (buscaRecursiva(fileIndice, chaveBuscada, rrnRaiz, &rrnResIndice, &ponteiroResDados)) {
            fseek(fileDados, ponteiroResDados, SEEK_SET);
            char removido;
            fread(&removido, sizeof(char), 1, fileDados);
            
            if (removido == '0') {
                // Remove fisicamente o nó do arquivo de dados e atualiza o encadeamento
                fseek(fileDados, ponteiroResDados, SEEK_SET);
                char flag = '1';
                fwrite(&flag, sizeof(char), 1, fileDados);
                fwrite(&topoDados, sizeof(int), 1, fileDados);
                
                int rrnRemovido = (ponteiroResDados - TAM_CABECALHO) / TAM_REG;
                topoDados = rrnRemovido;
                
                fseek(fileDados, 1, SEEK_SET);
                fwrite(&topoDados, sizeof(int), 1, fileDados);

                // Dispara a remoção de baixo nível na B-Tree
                removerChaveIndice(fileIndice, chaveBuscada);
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