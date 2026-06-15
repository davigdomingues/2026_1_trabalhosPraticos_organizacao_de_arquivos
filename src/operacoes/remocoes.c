#include "../../headers/operacoes/remocoes.h"
#include "../../headers/operacoes/buscas.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/utils.h"
#include "../../headers/fornecidas.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void handleDeleteWhere(){
    char *arquivoDados = NULL;
    FILE *fileDados = NULL;
    bool ok = false;

    arquivoDados = lerNomeArquivo();
    if (!arquivoDados) return;

    fileDados = fopen(arquivoDados, "r+b");
    if (!fileDados) {
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados);
        return;
    }

    // Validação de consistência do cabeçalho
    char statusDelSeq;
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fread(&statusDelSeq, sizeof(char), 1, fileDados);
    if (statusDelSeq != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados);
        free(arquivoDados);
        return;
    }

    // Trava o arquivo definindo status como inconsistente para o lote de operações
    char statusInconsistenteDel = '0';
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fwrite(&statusInconsistenteDel, sizeof(char), 1, fileDados);

    int nRemocoes = 0;
    scanf("%d", &nRemocoes);

    CampoValor *paresDelete = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES); // pares da remoção atual
    ok = true;
    // loop operações de deleção
    for (int i = 0; i < nRemocoes; i++) {
        int mPares = 0;
        scanf("%d", &mPares);
        
        lerPares(paresDelete, mPares); // leitura dos pares para a operação de remoção atual

        if (ok && !deleteWhere(fileDados, paresDelete, mPares)) {
            ok = false; // como falhou, não tenta as próximas
        }

        liberarPares(paresDelete, mPares); // liberação dos pares da operação de remoção atual antes de ler os próximos, para evitar acúmulo de memória alocada
    }
    free(paresDelete);

    // Recálculo e sincronização dos contadores uma única vez ao término de todas as remoções
    if (ok) {
        recalcularContadores(fileDados);
        
        char statusConsistente = '1';
        fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
        fwrite(&statusConsistente, sizeof(char), 1, fileDados);
    }
    fclose(fileDados);
    BinarioNaTela(arquivoDados);
    free(arquivoDados);
}

void handleDeleteWhereIndexado(){
    char *arquivoDados = NULL;
    char *arquivoIndice = NULL;
    FILE *fileDados = NULL;
    FILE *fileIndice = NULL;
    bool ok = false;

    arquivoDados = lerNomeArquivo();
    if (!arquivoDados){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    arquivoIndice = lerNomeArquivo();
    if (!arquivoIndice){
        printf("Falha no processamento do arquivo.\n");
        free(arquivoDados);
        return;
    }

    fileDados = fopen(arquivoDados, "r+b");
    fileIndice = fopen(arquivoIndice, "r+b");
    if (!fileDados || !fileIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (fileDados) fclose(fileDados);
        if (fileIndice) fclose(fileIndice);
        free(arquivoDados); free(arquivoIndice);
        return;
    }

    // Processo padrão de leitura e de validação do status dos arquivos
    char statusDadosDel, statusIndiceDel;

    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fread(&statusDadosDel, sizeof(char), 1, fileDados);

    fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
    fread(&statusIndiceDel, sizeof(char), 1, fileIndice);

    if (statusDadosDel != '1' || statusIndiceDel != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fileDados); fclose(fileIndice);
        free(arquivoDados); free(arquivoIndice);
        return;
    }

    // Processo de marcação dos arquivos como inconsistentes durante as operações
    char statusInconsistente = '0';
    fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
    fwrite(&statusInconsistente, sizeof(char), 1, fileDados);

    fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
    fwrite(&statusInconsistente, sizeof(char), 1, fileIndice);

    // Recuperação do número de nós do arquivo de índice para controle durante as remoções indexadas
    int nroNosRemocao;
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    fread(&nroNosRemocao, sizeof(int), 1, fileIndice);

    // Leitura do número de operações de remoção indexada a serem realizadas
    int nRemocoesIdx = 0;
    scanf("%d", &nRemocoesIdx);

    CampoValor *paresDeleteIdx = (CampoValor*) malloc(sizeof(CampoValor) * MAX_PARES);
    ok = true;

    // Loop das operações
    for (int i = 0; i < nRemocoesIdx; i++) {
        int mPares = 0;
        scanf("%d", &mPares);
        
        lerPares(paresDeleteIdx, mPares);

        if (ok && !deleteWhereIndexado(fileDados, fileIndice, paresDeleteIdx, mPares, &nroNosRemocao)) {
            ok = false;
        }

        liberarPares(paresDeleteIdx, mPares); 
    }
    free(paresDeleteIdx);

    // Atualização da contagem de nós no arquivo de índice
    fseek(fileIndice, BTREE_OFF_NRONOS, SEEK_SET);
    fwrite(&nroNosRemocao, sizeof(int), 1, fileIndice);

    // marcação de consistência
    if (ok) {
        char statusConsistente = '1';
        fseek(fileDados, DADOS_OFF_STATUS, SEEK_SET);
        fwrite(&statusConsistente, sizeof(char), 1, fileDados);
        
        fseek(fileIndice, BTREE_OFF_STATUS, SEEK_SET);
        fwrite(&statusConsistente, sizeof(char), 1, fileIndice);
    }

    fclose(fileDados);
    fclose(fileIndice);

    if (ok) {
        BinarioNaTela(arquivoDados);
        BinarioNaTela(arquivoIndice);
    }
    free(arquivoDados);
    free(arquivoIndice);
}


bool deleteWhere(FILE *fileDados, CampoValor *pares, int mPares) {
    int topoDados;

    // Recupera o topo atual da pilha de removidos
    fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET);
    if (fread(&topoDados, sizeof(int), 1, fileDados) != 1) return false;

    int rrn = -1;
    bool ok = true;
    // loop para marcar os registros como removidos e atualizar a lista de removidos no cabeçalho
    while(true){
        //busca o próximo registro que satisfaz os critérios de remoção
        rrn = selectWhere(fileDados, NULL, pares, mPares, rrn+1, true, true); //+1 para não ficar retornando sempre o mesmo rrn
        if(rrn < 0) break;

        long inicioRegistro = (long)TAM_CABECALHO + (long)rrn * (long)TAM_REG; // calcula o byte offset do registro a ser removido, usando o RRN para acessar diretamente
        if (fseek(fileDados, inicioRegistro, SEEK_SET) != 0) {  // posiciona o ponteiro no início do registro a ser removido
            ok = false; break; 
        }
        char removidoFlag = '1';

        if (fwrite(&removidoFlag, sizeof(char), 1, fileDados) != 1) {  // marca o registro como removido escrevendo '1' no byte de removido
            ok = false; break; 
        }
        
        if (fwrite(&topoDados, sizeof(int), 1, fileDados) != 1) {  // escreve o antigo topo da lista de removidos no campo de proxRRN do registro removido, para manter a lista encadeada
            ok = false; break; 
        }
        topoDados = rrn; // atualiza o topo para o RRN do registro recém-removido, que agora é o primeiro da lista de removidos
        // fseek(fileDados, inicioRegistro + TAM_REG, SEEK_SET);
    }

    if (ok) {
        // Atualiza de forma consolidada o novo topo da lista encadeada de lixo
        fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET);
        fwrite(&topoDados, sizeof(int), 1, fileDados);
    }

    return ok;
}

bool removerChaveIndice(FILE *fileIndice, int chave, int *nroNos) {
    int rrnRaiz;
    fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET); // lê o RRN da raiz da Árvore-B a partir do cabeçalho do arquivo de índice
    fread(&rrnRaiz, sizeof(int), 1, fileIndice); // se o RRN da raiz for -1, significa que a árvore está vazia e a chave não pode ser encontrada para remoção, retornando false imediatamente

    if (rrnRaiz == -1) return false;

    // chama a função recursiva de remoção, que retorna um status indicando se a chave foi encontrada e removida ou não
    int status = removerRecursivo(fileIndice, rrnRaiz, chave, nroNos);
    
    if (status == CHAVE_NAO_ENCONTRADA) {
        return false; 
    }

    // esvaziamento de raiz tratado somente após a remoção recursiva para evitar casos de underflow pendente na raiz
    No *raiz = inicializarNo();
    fseek(fileIndice, BTREE_NO_INICIO(rrnRaiz), SEEK_SET);
    lerNo(fileIndice, raiz);

    if (raiz->nroChaves == 0) { // Raiz ficou vazia, precisa ser esvaziada ou substituída pela sua única subárvore (se existir)
        // Usa a verificação de ponteiros para saber se é folha
        if (raiz->P[0] != -1) {
            int novaRaiz = raiz->P[0]; 
            
            // atualiza o RRN da raiz no cabeçalho do arquivo de índice para apontar para a nova raiz, promovendo essa subárvore para ser a nova raiz da Árvore-B
            fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
            fwrite(&novaRaiz, sizeof(int), 1, fileIndice);

            apagarNo(fileIndice, rrnRaiz, nroNos); // O nó antigo da raiz é logicamente apagado, mas seu espaço não é reutilizado para novas inserções, seguindo a política de alocação de nós do projeto.
            
            // após atualizar o RRN da nova raiz no cabeçalho, é necessário garantir que o tipo do nó seja atualizado para NO_RAIZ
            No *nRaiz = inicializarNo();
            fseek(fileIndice, BTREE_NO_INICIO(novaRaiz), SEEK_SET);
            lerNo(fileIndice, nRaiz);
            
            // Força a nova raiz a ter a flag 0, independentemente de ser folha ou não
            nRaiz->tipoNo = NO_RAIZ;
            
            // escreve a nova raiz de volta no arquivo para garantir que o tipo atualizado seja persistido, mantendo a consistência da estrutura da Árvore-B 
            fseek(fileIndice, BTREE_NO_INICIO(novaRaiz), SEEK_SET);
            escreverNo(fileIndice, nRaiz);
            free(nRaiz);

        } else { // Raiz é folha e ficou vazia, pode ser esvaziada normalmente
            int vazio = -1;

            // Atualiza o RRN da raiz para -1, indicando que a árvore agora está vazia
            fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
            fwrite(&vazio, sizeof(int), 1, fileIndice);
            apagarNo(fileIndice, rrnRaiz, nroNos);
        }
    }
    free(raiz);
    return true; 
}

int removerRecursivo(FILE *fileIndice, int rrnAtual, int chave, int *nroNos) {
    if (rrnAtual == -1) return CHAVE_NAO_ENCONTRADA;

    // Carrega o nó atual para análise
    No *noAtual = inicializarNo();
    fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET);
    lerNo(fileIndice, noAtual);

    int pos = 0;
    // Encontra onde a chave deveria estar (chave ou ponteiro)
    while (pos < noAtual->nroChaves && noAtual->C[pos] < chave) pos++;

    // Verifica se a chave foi encontrada
    bool encontrada = (pos < noAtual->nroChaves && noAtual->C[pos] == chave);

    if (encontrada) {
        if (noAtual->P[0] == -1) { // A chave a ser removida está em uma folha
            // Remoção via shift
            for (int i = pos; i < noAtual->nroChaves - 1; i++) {
                noAtual->C[i] = noAtual->C[i+1];
                noAtual->Pr[i] = noAtual->Pr[i+1];
            }

            // consistência dos dados mantida
            noAtual->nroChaves--;
            noAtual->C[noAtual->nroChaves] = -1;
            noAtual->Pr[noAtual->nroChaves] = -1;            
        } else { // A chave a ser removida está em um nó interno
            // Substituição pela chave sucessora em nós internos
            int rrnSucessor = noAtual->P[pos+1];
            No *sucessor = inicializarNo();
            fseek(fileIndice, BTREE_NO_INICIO(rrnSucessor), SEEK_SET);
            lerNo(fileIndice, sucessor);
            
            // Desce pela subárvore da direita para encontrar o sucessor
            while (sucessor->P[0] != -1) {
                rrnSucessor = sucessor->P[0];
                fseek(fileIndice, BTREE_NO_INICIO(rrnSucessor), SEEK_SET);
                lerNo(fileIndice, sucessor);
            }
            
            // O sucessor encontrado é o menor elemento da subárvore direita
            int chaveSucessora = sucessor->C[0];
            int ponteiroDadosSucessor = sucessor->Pr[0];
            free(sucessor);

            // Promove o sucessor para a posição da chave a ser removida
            noAtual->C[pos] = chaveSucessora;
            noAtual->Pr[pos] = ponteiroDadosSucessor;
            fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET);
            escreverNo(fileIndice, noAtual);

            // Chamada para apagar a sucessora promovida
            int statusSub = removerRecursivo(fileIndice, noAtual->P[pos+1], chaveSucessora, nroNos);
            if (statusSub == UNDERFLOW_PENDENTE) {
                tratarUnderflow(fileIndice, noAtual, rrnAtual, pos + 1, nroNos);
            }
        }
    } else { // A chave não foi encontrada no nó atual
        if (noAtual->P[0] == -1) {
            free(noAtual);
            return CHAVE_NAO_ENCONTRADA; 
        }

        // Continua a busca na subárvore adequada
        int statusSub = removerRecursivo(fileIndice, noAtual->P[pos], chave, nroNos);
        if (statusSub == CHAVE_NAO_ENCONTRADA) {
            free(noAtual);
            return CHAVE_NAO_ENCONTRADA;
        }

        // Após a chamada recursiva, verifica se houve underflow pendente
        if (statusSub == UNDERFLOW_PENDENTE) {
            tratarUnderflow(fileIndice, noAtual, rrnAtual, pos, nroNos);
        }
    }

    // Escreve as alterações de volta no arquivo para o nó atual após a remoção ou promoção
    fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET);
    escreverNo(fileIndice, noAtual);

    int chavesAtuais = noAtual->nroChaves;
    free(noAtual);

    // Verificação de underflow
    if (chavesAtuais < MIN_CHAVES) {
        return UNDERFLOW_PENDENTE;
    }

    return SUCESSO;
}

void tratarUnderflow(FILE *fileIndice, No *pai, int rrnPai, int indicePonteiroFilho, int *nroNos) {
    // Carrega o nó em underflow e seus irmãos adjacentes (se existirem)
    int rrnAtual = pai->P[indicePonteiroFilho]; // RRN do nó que está em underflow
    No *atual = inicializarNo(); 
    fseek(fileIndice, TAM_BTREE_CABECALHO + rrnAtual * TAM_NO, SEEK_SET); 
    lerNo(fileIndice, atual); // nó em underflow
    
    No *irmEsq = NULL; int rrnIrmEsq = -1;
    if (indicePonteiroFilho > 0) {
        rrnIrmEsq = pai->P[indicePonteiroFilho - 1]; 
        irmEsq = inicializarNo(); fseek(fileIndice, BTREE_NO_INICIO(rrnIrmEsq), SEEK_SET); 
        lerNo(fileIndice, irmEsq);
    }

    No *irmDir = NULL; // pode ser necessário para empréstimo ou merge, mas só é carregado se existir
    int rrnIrmDir = -1;

    // Carrega o irmão direito somente se existir, para evitar leituras desnecessárias
    if (indicePonteiroFilho < pai->nroChaves) {
        rrnIrmDir = pai->P[indicePonteiroFilho + 1]; 
        irmDir = inicializarNo(); 
        fseek(fileIndice, BTREE_NO_INICIO(rrnIrmDir), SEEK_SET); 
        lerNo(fileIndice, irmDir);
    }

    // Empréstimo na direita, se o irmão direito tiver chaves suficientes para emprestar
    if (irmDir != NULL && irmDir->nroChaves > MIN_CHAVES) {
        int chavesTotal = 1 + irmDir->nroChaves;
        int chavesEsq = chavesTotal / 2; // nó mais à esquerda deverá conter uma chave a mais
        int chavesDir = chavesTotal - chavesEsq - 1;

        int bufC[4], bufPr[4], bufP[5]; // buffers temporários para reorganizar as chaves e ponteiros durante o empréstimo
        bufC[0] = pai->C[indicePonteiroFilho]; bufPr[0] = pai->Pr[indicePonteiroFilho]; bufP[0] = atual->P[0];

        // Preenche os buffers com as chaves e ponteiros do irmão direito
        for(int i = 0; i < irmDir->nroChaves; i++) {
            bufC[i+1] = irmDir->C[i]; 
            bufPr[i+1] = irmDir->Pr[i]; 
            bufP[i+1] = irmDir->P[i];
        }

        bufP[irmDir->nroChaves + 1] = irmDir->P[irmDir->nroChaves];

        // Reorganiza o nó atual
        atual->nroChaves = chavesEsq;
        for(int i = 0; i < chavesEsq; i++) { 
            atual->C[i] = bufC[i]; 
            atual->Pr[i] = bufPr[i]; 
            atual->P[i] = bufP[i]; 
        }
        atual->P[chavesEsq] = bufP[chavesEsq];

        pai->C[indicePonteiroFilho] = bufC[chavesEsq]; pai->Pr[indicePonteiroFilho] = bufPr[chavesEsq];

        // O irmão direito recebe as chaves restantes após a chave promovida para o pai
        irmDir->nroChaves = chavesDir;
        for(int i = 0; i < chavesDir; i++) {
            irmDir->C[i] = bufC[chavesEsq + 1 + i]; 
            irmDir->Pr[i] = bufPr[chavesEsq + 1 + i]; 
            irmDir->P[i] = bufP[chavesEsq + 1 + i];
        }
    
        // O ponteiro mais à direita do irmão direito é atualizado
        irmDir->P[chavesDir] = bufP[chavesEsq + 1 + chavesDir];
        for(int i = chavesDir; i < 3; i++) { 
            irmDir->C[i] = -1; 
            irmDir->Pr[i] = -1; 
            irmDir->P[i+1] = -1; 
        }

        // Escreve as alterações de volta no arquivo para o pai, o nó em underflow e o irmão direito
        fseek(fileIndice, BTREE_NO_INICIO(rrnPai), SEEK_SET); escreverNo(fileIndice, pai);
        fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET); escreverNo(fileIndice, atual);
        fseek(fileIndice, BTREE_NO_INICIO(rrnIrmDir), SEEK_SET); escreverNo(fileIndice, irmDir);

        free(atual); 
        
        if(irmEsq) free(irmEsq); 
        free(irmDir); 
        
        return;
    }

    // Empréstimo na esquerda, se o irmão esquerdo tiver chaves suficientes para emprestar
    if (irmEsq != NULL && irmEsq->nroChaves > MIN_CHAVES) {
        int chavesTotal = irmEsq->nroChaves + 1;
        int chavesEsq = chavesTotal / 2;
        int chavesDir = chavesTotal - chavesEsq - 1;

        int bufC[4], bufPr[4], bufP[5];

        // Preenche os buffers com as chaves e ponteiros do irmão esquerdo, mantendo a ordem original
        for(int i = 0; i < irmEsq->nroChaves; i++) {
            bufC[i] = irmEsq->C[i]; 
            bufPr[i] = irmEsq->Pr[i]; 
            bufP[i] = irmEsq->P[i];
        }

        // O ponteiro mais à direita do irmão esquerdo é movido para o nó em underflow, e a chave do pai que separa os dois irmãos 
        // é promovida para o nó em underflow
        bufP[irmEsq->nroChaves] = irmEsq->P[irmEsq->nroChaves];
        bufC[irmEsq->nroChaves] = pai->C[indicePonteiroFilho - 1]; bufPr[irmEsq->nroChaves] = pai->Pr[indicePonteiroFilho - 1];
        bufP[irmEsq->nroChaves + 1] = atual->P[0];

        // Reorganização das chaves
        irmEsq->nroChaves = chavesEsq;
        for(int i = 0; i < chavesEsq; i++) { 
            irmEsq->C[i] = bufC[i]; 
            irmEsq->Pr[i] = bufPr[i]; 
            irmEsq->P[i] = bufP[i]; 
        }

        // Atualização de ponteiros do irmão esquerdo após o empréstimo
        irmEsq->P[chavesEsq] = bufP[chavesEsq];
        for(int i = chavesEsq; i < 3; i++) { 
            irmEsq->C[i] = -1; 
            irmEsq->Pr[i] = -1; 
            irmEsq->P[i+1] = -1; 
        }

        pai->C[indicePonteiroFilho - 1] = bufC[chavesEsq]; pai->Pr[indicePonteiroFilho - 1] = bufPr[chavesEsq];

        // O nó em underflow recebe as chaves restantes
        atual->nroChaves = chavesDir;
        for(int i = 0; i < chavesDir; i++) {
            atual->C[i] = bufC[chavesEsq + 1 + i]; 
            atual->Pr[i] = bufPr[chavesEsq + 1 + i]; 
            atual->P[i] = bufP[chavesEsq + 1 + i];
        }
        atual->P[chavesDir] = bufP[chavesEsq + 1 + chavesDir];

        fseek(fileIndice, BTREE_NO_INICIO(rrnPai), SEEK_SET); escreverNo(fileIndice, pai);
        fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET); escreverNo(fileIndice, atual);
        fseek(fileIndice, BTREE_NO_INICIO(rrnIrmEsq), SEEK_SET); escreverNo(fileIndice, irmEsq);

        free(atual); 
        free(irmEsq); 
        if(irmDir) free(irmDir); 
        
        return;
    }

    // Merge na esquerda
    if (irmEsq != NULL)
        fazerMerge(fileIndice, irmEsq, atual, pai, rrnIrmEsq, rrnAtual, rrnPai, indicePonteiroFilho - 1, nroNos);

    // Merge na direita
    else if (irmDir != NULL)
        fazerMerge(fileIndice, atual, irmDir, pai, rrnAtual, rrnIrmDir, rrnPai, indicePonteiroFilho, nroNos);
    
    free(atual); 
    if(irmEsq) free(irmEsq); 
    if(irmDir) free(irmDir);
}

void fazerMerge(FILE *fileIndice, No *esq, No *dir, No *pai, int rrnEsq, int rrnDir, int rrnPai, int indiceChavePai, int *nroNos) {
    // A chave do pai que separa os dois nós é movida para o nó da esquerda
    // e as chaves do nó da direita são anexadas à direita do nó da esquerda
    esq->C[esq->nroChaves] = pai->C[indiceChavePai];
    esq->Pr[esq->nroChaves] = pai->Pr[indiceChavePai];
    esq->nroChaves++;

    // ordem original mantida durante a anexação das chaves
    for (int i = 0; i < dir->nroChaves; i++) {
        esq->C[esq->nroChaves + i] = dir->C[i];
        esq->Pr[esq->nroChaves + i] = dir->Pr[i];
        esq->P[esq->nroChaves + i] = dir->P[i];
    }
    esq->P[esq->nroChaves + dir->nroChaves] = dir->P[dir->nroChaves];
    esq->nroChaves += dir->nroChaves;

    // Após o merge, a chave que separava os dois nós no pai é removida, e os ponteiros são ajustados
    for (int i = indiceChavePai; i < pai->nroChaves - 1; i++) {
        pai->C[i] = pai->C[i+1];
        pai->Pr[i] = pai->Pr[i+1];
        pai->P[i+1] = pai->P[i+2];
    }

    // O número de chaves do pai é decrementado, e os campos restantes são limpos
    pai->nroChaves--;
    pai->C[pai->nroChaves] = -1;
    pai->Pr[pai->nroChaves] = -1;
    pai->P[pai->nroChaves + 1] = -1;

    // Escreve as alterações de volta no arquivo para o pai e o nó esquerdo resultante
    fseek(fileIndice, BTREE_NO_INICIO(rrnEsq), SEEK_SET); escreverNo(fileIndice, esq);
    fseek(fileIndice, BTREE_NO_INICIO(rrnPai), SEEK_SET); escreverNo(fileIndice, pai);
    
    apagarNo(fileIndice, rrnDir, nroNos); // O nó da direita é logicamente apagado
}

bool deleteWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares, int *nroNos) {
    if (!fileDados || !fileIndice) return false;

    char statusDados, statusIndice;
    int topoDados;

    // Pelo status na main, apenas realiza a leitura do topo o qual será usado
    fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET);
    if (fread(&topoDados, sizeof(int), 1, fileDados) != 1) return false;

    bool ok = true;
    int idxCodEstacao = encontrarIndexCampo(pares, mPares, "codEstacao");

    if (idxCodEstacao != -1 && !valorEhNulo(pares[idxCodEstacao].valor)) {
        // Busca por índices da Árvore-B, seguida de validação completa dos filtros no registro encontrado
        int chaveBuscada = atoi(pares[idxCodEstacao].valor);
        int rrnRaiz; fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET); fread(&rrnRaiz, sizeof(int), 1, fileIndice);

        int rrnResIndice, ponteiroResDados;
        // Encontrou a chave no índice, agora valida os outros filtros no registro correspondente antes de marcar como removido
        if (buscaRecursiva(fileIndice, chaveBuscada, rrnRaiz, &rrnResIndice, &ponteiroResDados)) {
            fseek(fileDados, ponteiroResDados, SEEK_SET);
            char removido; fread(&removido, sizeof(char), 1, fileDados);
                
            if (removido == '0') { // Registro encontrado e não removido
                int rrnAlvo = (ponteiroResDados - TAM_CABECALHO) / TAM_REG;
                    
                // Valida se os outros filtros (nome, linha, etc) também batem no registro encontrado
                int rrnConfirmado = selectWhere(fileDados, NULL, pares, mPares, rrnAlvo, true, true);
                    
                if (rrnConfirmado == rrnAlvo) { // Todos os filtros batem, pode-se marcar como removido
                    fseek(fileDados, ponteiroResDados, SEEK_SET); 
                    char flag = '1'; // Marca como removido
                    fwrite(&flag, sizeof(char), 1, fileDados); 
                    fwrite(&topoDados, sizeof(int), 1, fileDados);
                    
                    // Atualiza o topo da lista de removidos no arquivo de dados
                    topoDados = rrnAlvo;
                    fseek(fileDados, 1, SEEK_SET); 
                    fwrite(&topoDados, sizeof(int), 1, fileDados);
                        
                    removerChaveIndice(fileIndice, chaveBuscada, nroNos);
                }
            }
        }
    } else {
        // Busca sequencial, lendo diretamente no disco
        int rrn = -1;
        while (true) {
            // Passamos NULL para o índice, forçando o "full-scan" no selectWhere
            rrn = selectWhere(fileDados, NULL, pares, mPares, rrn + 1, true, true);
            if (rrn < 0) break;

            long inicioRegistro = (long)TAM_CABECALHO + (long)rrn * (long)TAM_REG;
            
            // Lê a chave (codEstacao) da estrutura do registro antes de marcar como removido
            fseek(fileDados, inicioRegistro + DADOS_OFF_PROXRRN, SEEK_SET); 
            int chaveParaRemover;
            if (fread(&chaveParaRemover, sizeof(int), 1, fileDados) != 1) { ok = false; break; }

            // Marca o registro como removido
            fseek(fileDados, inicioRegistro, SEEK_SET);
            char flag = '1';
            if (fwrite(&flag, sizeof(char), 1, fileDados) != 1) { 
                ok = false; 
                break; 
            }

            // Escreve o antigo topo da lista de removidos no campo de proxRRN do registro removido, para manter a lista encadeada
            if (fwrite(&topoDados, sizeof(int), 1, fileDados) != 1) { 
                ok = false; 
                break; 
            }
            
            topoDados = rrn;

            // Remove a chave resgatada da Árvore-B
            if (!removerChaveIndice(fileIndice, chaveParaRemover, nroNos)) {
                ok = false; 
                break;
            }
        }
        if (ok) { // Após a remoção lógica de todos os registros que batem com os filtros, atualiza o topo da lista de removidos
            fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET);
            fwrite(&topoDados, sizeof(int), 1, fileDados);
        }
    }

    // Atualização dos dados persistentes
    recalcularContadores(fileDados);

    return ok;
}