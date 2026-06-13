#include "../../headers/operacoes/insercoes.h"
#include "../../headers/operacoes/buscas.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/indice/btree_no.h"
#include "../../headers/utils.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void distribuirUniforme(FILE *fileIndice, No *no, No *novoNo, int rrnNovoNo, ElementoIndice overflowElem, ElementoIndice *promoPraCima){
    ElementoIndice elems[4];

    elems[0] = (ElementoIndice){no->C[0], no->Pr[0], no->P[1]};
    elems[1] = (ElementoIndice){no->C[1], no->Pr[1], no->P[2]};
    elems[2] = (ElementoIndice){no->C[2], no->Pr[2], no->P[3]};
    elems[3] = overflowElem;

    int ponteiroMaisEsquerda = no->P[0];

    insertionSort(elems, 4);

    no->C[0] = elems[0].chave;
    no->Pr[0] = elems[0].ptrDados;
    no->P[1] = elems[0].filhoDir;

    no->C[1] = elems[1].chave;
    no->Pr[1] = elems[1].ptrDados;
    no->P[2] = elems[1].filhoDir;

    no->C[2] = -1;
    no->Pr[2] = -1;
    no->P[3] = -1;
    no->P[0] = ponteiroMaisEsquerda;
    no->nroChaves = 2;

    promoPraCima->chave = elems[2].chave;
    promoPraCima->ptrDados = elems[2].ptrDados;
    promoPraCima->filhoDir = rrnNovoNo;

    novoNo->C[0] = elems[3].chave;
    novoNo->Pr[0]= elems[3].ptrDados;
    novoNo->P[0] = elems[2].filhoDir;
    novoNo->P[1] = elems[3].filhoDir;

    novoNo->C[1] = -1; novoNo->C[2]  = -1;
    novoNo->Pr[1] = -1; novoNo->Pr[2] = -1;
    novoNo->P[2] = -1; novoNo->P[3]  = -1;
    
    novoNo->nroChaves = 1;
    novoNo->tipoNo = no->tipoNo;
}


bool insereOrdenado(No *no, ElementoIndice elem){
    if(no->nroChaves >= NRO_MAX_CHAVES) return false;

    int i = no->nroChaves - 1;
    //shifta todos os elementos maiores do que a chave para a direita
    while (i >= 0 && no->C[i] > elem.chave) {
        no->C[i+1] = no->C[i];
        no->Pr[i+1] = no->Pr[i];
        no->P[i+2] = no->P[i+1]; //o filho a direita acompanha a chave
        i--;
    }

    //insere na posição correta
    int posicaoInsercao = i + 1;
    no->C[posicaoInsercao] = elem.chave;
    no->Pr[posicaoInsercao] = elem.ptrDados;
    no->P[posicaoInsercao+1] = elem.filhoDir;

    no->nroChaves++;
    return true;
}


No *split(FILE *fileIndice, No *no, ElementoIndice overflowElem, ElementoIndice *promoPraCima){
    int rrnNovoNo;
    No *novoNo = criarNo(fileIndice, &rrnNovoNo);

    // Se a página que estourou era a RAIZ (0), ela perde a "coroa" e vira um nó intermediário, e o novo nó criado também é intermediário
    if (no->tipoNo == NO_RAIZ) {
        no->tipoNo = NO_INTERMEDIARIO;
        novoNo->tipoNo = NO_INTERMEDIARIO;
    } else {
        // Se era Folha (-1) ou já era intermediário (1), apenas copia
        novoNo->tipoNo = no->tipoNo; 
    }

    // distribui os elementos de forma uniforme entre o nó que estourou e o novo nó criado, e define o elemento a ser promovido para o pai
    distribuirUniforme(fileIndice, no, novoNo, rrnNovoNo, overflowElem, promoPraCima);
    return novoNo;
}

void criarNovaRaiz(FILE *fileIndice, int rrnRaizAtual, ElementoIndice promoDeBaixo, int *nroNos){
    int rrnNovaRaiz;
    No *novaRaiz = criarNo(fileIndice, &rrnNovaRaiz);

    novaRaiz->nroChaves = 1;
    novaRaiz->C[0] = promoDeBaixo.chave;
    novaRaiz->Pr[0] = promoDeBaixo.ptrDados;
    novaRaiz->P[0] = rrnRaizAtual;          //filho a esquerda é a raiz antiga
    novaRaiz->P[1] = promoDeBaixo.filhoDir; //filho a direita é o nó criado no split
    novaRaiz->tipoNo = NO_RAIZ;

    //atualiza o cabeçalho do arquivo para apontar para o RRN da nova raiz
    fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET); 
    fwrite(&rrnNovaRaiz, sizeof(int), 1, fileIndice);

    //se este for ser o único nó da árvore, 
    //quer dizer que é um nó raiz e folha.
    //por convenção, o tipo é folha
    if(*nroNos == 0) novaRaiz->tipoNo = NO_FOLHA;
    (*nroNos)++;

    //salva a nova raiz no arquivo
    int inicioNovaRaiz = BTREE_NO_INICIO(rrnNovaRaiz);
    fseek(fileIndice, inicioNovaRaiz, SEEK_SET);
    escreverNo(fileIndice, novaRaiz);
        
    free(novaRaiz);
}

int insertIndice(FILE *fileIndice, int chave, int ptrDados, int *nroNos){
    char inconsistente = '0';
    fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, fileIndice);

    int rrnRaiz;
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    ElementoIndice promoDeBaixo;
    int res = insertIndiceRec(fileIndice, chave, ptrDados, rrnRaiz, &promoDeBaixo, nroNos);
    if(res == PROMOCAO) criarNovaRaiz(fileIndice, rrnRaiz, promoDeBaixo, nroNos);

    //atualiza o status de consistência
    fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
    char consistente = '1';
    fwrite(&consistente, sizeof(char), 1, fileIndice);
    return 1;
}

int insertIndiceRec(FILE *fileIndice, int chave, int ptrDados, int rrnNoAtual, ElementoIndice *promoPraCima, int *nroNovosNos){
    if(rrnNoAtual == -1){
        promoPraCima->chave = chave;
        promoPraCima->ptrDados = ptrDados;
        promoPraCima->filhoDir = -1;
        return PROMOCAO;
    }
    int inicioNo = BTREE_NO_INICIO(rrnNoAtual);
    fseek(fileIndice, inicioNo, SEEK_SET);

    No *no = inicializarNo();
    lerNo(fileIndice, no);

    int subArvore;
    int dummy; //não vai ser utilizado
    bool encontrou = encontrarChave(no, chave, &subArvore, &dummy);
    if(encontrou) {
        free(no);
        return ERRO_DE_INSERCAO;
    }

    ElementoIndice promoDeBaixo;
    int res = insertIndiceRec(fileIndice, chave, ptrDados, subArvore, &promoDeBaixo, nroNovosNos);

    if(res == SEM_PROMOCAO || res == ERRO_DE_INSERCAO) {
        free(no);
        return res;
    }
    else if (no->nroChaves < NRO_MAX_CHAVES){
        insereOrdenado(no, promoDeBaixo);

        fseek(fileIndice, inicioNo, SEEK_SET);
        escreverNo(fileIndice, no);
        
        free(no);
        return SEM_PROMOCAO;
    } else {

        No *novoNo = split(fileIndice, no, promoDeBaixo, promoPraCima);

        fseek(fileIndice, inicioNo, SEEK_SET);
        escreverNo(fileIndice, no);

        int inicioNovoNo = BTREE_NO_INICIO(promoPraCima->filhoDir);
        fseek(fileIndice, inicioNovoNo, SEEK_SET);
        escreverNo(fileIndice, novoNo);

        (*nroNovosNos)++;
        
        free(no);
        free(novoNo);
        
        return PROMOCAO;
    }

    return 0;
}

bool insert(FILE *fileDados, FILE *fileIndice, CampoValor *valores, int mValores, int *nroNos) {
    // Se o campo de código da estação estiver presente nos valores a serem inseridos e o arquivo de índice existir, verifica se já existe um registro com o mesmo código de estação usando o índice
    if(*valores[CAMPO_COD_ESTACAO].valor && fileIndice != NULL){
        CampoValor *pares[8] = {NULL};
        CampoValor *apenasCodEstacao = (CampoValor*) malloc(sizeof(CampoValor));
        apenasCodEstacao->campo = valores[CAMPO_COD_ESTACAO].campo;
        apenasCodEstacao->valor = valores[CAMPO_COD_ESTACAO].valor;
        pares[0] = apenasCodEstacao;

        // Se a busca retornar um resultado, significa que já existe um registro com o mesmo código de estação, e a inserção deve ser abortada para evitar duplicidade
        int res = selectWhereIndexado(fileDados, fileIndice, pares, 1, false);
        if(res > 0){
            free(apenasCodEstacao);
            return true;
        }
    }

    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroPares;

    // lê o status, o topo da lista de removidos e os contadores do cabeçalho
    if (fread(&status, sizeof(char), 1, fileDados) != 1 || status != '1' ||
        fread(&topo, sizeof(int), 1, fileDados) != 1 ||
        fread(&proxRRN, sizeof(int), 1, fileDados) != 1 ||
        fread(&nroEstacoes, sizeof(int), 1, fileDados) != 1 ||
        fread(&nroPares, sizeof(int), 1, fileDados) != 1) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    // atualiza o status para '0' para indicar que o arquivo está sendo modificado e volta o ponteiro para o início do arquivo para depois atualizar os contadores no cabeçalho
    atualizarStatus(fileDados, '0', true);

    Registro reg = inicializarReg();
    int rrnNovoReg = -1;
    int ponteiroDados;
    bool ok = true;

    // aplica os pares usando utilitário compartilhado (conversão + NULO + strings)
    if (!aplicarParesEmRegistro(&reg, valores, mValores)) ok = false;

    if (ok) {
        if (TAM_LIVRE_REG(reg.tamNomeEstacao, reg.tamNomeLinha) < 0) ok = false;

        if (topo != -1) {
            // Reaproveita um registro removido.
            long off = (long)TAM_CABECALHO + (long)topo * (long)TAM_REG;
            ponteiroDados = off;
            rrnNovoReg = off;
            int proximoTopo = -1;

            // lê o próximo da lista de removidos para atualizar o topo depois
            if (fseek(fileDados, off + 1, SEEK_SET) != 0 || fread(&proximoTopo, sizeof(int), 1, fileDados) != 1) {
                ok = false;
            } else if (fseek(fileDados, off, SEEK_SET) != 0) { // posiciona para escrever o registro no lugar do removido
                ok = false;
            } else { // escreve o registro no lugar do removido
                escreverReg(fileDados, &reg);
                // atualiza topo para o próximo da lista de removidos
                if (fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET) != 0 || fwrite(&proximoTopo, sizeof(int), 1, fileDados) != 1) ok = false;
            }

        } else {
            // Sem removidos: escreve no fim (append)
            if (fseek(fileDados, 0, SEEK_END) != 0) ok = false;

            // escreve o novo registro no fim do arquivo e incrementa proxRRN para apontar para o próximo registro a ser inserido
            if (ok) {
                rrnNovoReg = proxRRN;
                ponteiroDados = (long)TAM_CABECALHO + (long)rrnNovoReg * (long)TAM_REG;
                escreverReg(fileDados, &reg);
                // somente incrementa o proxRRN quando escreve no fim do arquivo
                proxRRN++;
            }
        }

        // salva o proxRRN atualizado no cabeçalho (sempre salva, pois ele pode ter mudado no else acima)
        fseek(fileDados, DADOS_OFF_PROXRRN, SEEK_SET);
        fwrite(&proxRRN, sizeof(int), 1, fileDados);
        atualizarStatus(fileDados, '1', true);
    }

    if (reg.tamNomeEstacao > 0) free(reg.nomeEstacao);
    if (reg.tamNomeLinha > 0) free(reg.nomeLinha);


    // caso haja algum erro durante a escrita do registro ou a atualização dos contadores
    if (!ok) {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    // lógica específica para o arquivo de índice: se ele existir, insere o par chave-ponteiro no índice
    // e trata a questão da consistência do arquivo de índice durante a escrita
    if (fileIndice != NULL) {
        if (!fileIndice) {
            ok = false;
        } else {
            insertIndice(fileIndice, reg.codEstacao, ponteiroDados, nroNos);
        }

        if (!ok) printf("Falha no processamento do arquivo.\n");
    }
    return ok;
}