#include "../../headers/operacoes/remocoes.h"
#include "../../headers/operacoes/buscas.h"
#include "../../headers/dados/registro.h"
#include "../../headers/dados/cabecalho.h"
#include "../../headers/indice/btree_cabecalho.h"
#include "../../headers/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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
    fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
    fread(&rrnRaiz, sizeof(int), 1, fileIndice);

    if (rrnRaiz == -1) return false;

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
            
            fseek(fileIndice, BTREE_OFF_NORAIZ, SEEK_SET);
            fwrite(&novaRaiz, sizeof(int), 1, fileIndice);

            apagarNo(fileIndice, rrnRaiz, nroNos); // O nó antigo da raiz é logicamente apagado, mas seu espaço não é reutilizado para novas inserções, seguindo a política de alocação de nós do projeto.
            
            No *nRaiz = inicializarNo();
            fseek(fileIndice, BTREE_NO_INICIO(novaRaiz), SEEK_SET);
            lerNo(fileIndice, nRaiz);
            
            // Força a nova raiz a ter a flag 0, independentemente de ser folha ou não
            nRaiz->tipoNo = NO_RAIZ;
            
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

    No *noAtual = inicializarNo();
    fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET);
    lerNo(fileIndice, noAtual);

    int pos = 0;
    // Encontra a posição onde a chave deveria estar, ou o ponteiro para a subárvore onde ela deveria estar se não for encontrada no nó atual
    while (pos < noAtual->nroChaves && noAtual->C[pos] < chave) pos++;

    // Verifica se a chave foi encontrada na posição identificada
    bool encontrada = (pos < noAtual->nroChaves && noAtual->C[pos] == chave);

    if (encontrada) {
        if (noAtual->P[0] == -1) { // A chave a ser removida está em uma folha, pode ser removida diretamente sem necessidade de promoção
            // Remoção física do registro na folha via shift (contiguidade mantida)
            for (int i = pos; i < noAtual->nroChaves - 1; i++) {
                noAtual->C[i] = noAtual->C[i+1];
                noAtual->Pr[i] = noAtual->Pr[i+1];
            }

            // Decrementa o número de chaves do nó e limpa os campos restantes para manter a consistência dos dados
            noAtual->nroChaves--;
            noAtual->C[noAtual->nroChaves] = -1;
            noAtual->Pr[noAtual->nroChaves] = -1;            
        } else { // A chave a ser removida está em um nó interno, precisa ser promovida para manter as propriedades da Árvore-B
            // Substituição pela chave sucessora em nós internos
            int rrnSucessor = noAtual->P[pos+1];
            No *sucessor = inicializarNo();
            fseek(fileIndice, BTREE_NO_INICIO(rrnSucessor), SEEK_SET);
            lerNo(fileIndice, sucessor);
            
            // Desce pela subárvore da direita para encontrar o sucessor in-order, que é o menor elemento dessa subárvore
            while (sucessor->P[0] != -1) {
                rrnSucessor = sucessor->P[0];
                fseek(fileIndice, BTREE_NO_INICIO(rrnSucessor), SEEK_SET);
                lerNo(fileIndice, sucessor);
            }
            
            // O sucessor encontrado é o menor elemento da subárvore direita, que pode ser promovido para substituir a chave a ser removida no nó atual
            int chaveSucessora = sucessor->C[0];
            int ponteiroDadosSucessor = sucessor->Pr[0];
            free(sucessor);

            // Promove o sucessor para a posição da chave a ser removida no nó atual
            noAtual->C[pos] = chaveSucessora;
            noAtual->Pr[pos] = ponteiroDadosSucessor;
            fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET);
            escreverNo(fileIndice, noAtual);

            // Chamada recursiva para apagar a sucessora promovida
            int statusSub = removerRecursivo(fileIndice, noAtual->P[pos+1], chaveSucessora, nroNos);
            if (statusSub == UNDERFLOW_PENDENTE) {
                tratarUnderflow(fileIndice, noAtual, rrnAtual, pos + 1, nroNos);
            }
        }
    } else { // A chave não foi encontrada no nó atual, continua a busca na subárvore adequada
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

        // Após a chamada recursiva, verifica se houve underflow pendente na subárvore e trata adequadamente para garantir as propriedades da Árvore-B
        if (statusSub == UNDERFLOW_PENDENTE) {
            tratarUnderflow(fileIndice, noAtual, rrnAtual, pos, nroNos);
        }
    }

    // Escreve as alterações de volta no arquivo para o nó atual após a remoção ou promoção
    fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET);
    escreverNo(fileIndice, noAtual);

    int chavesAtuais = noAtual->nroChaves;
    free(noAtual);

    // Verificação de underflow, em que caso o número de chaves do nó atual ficou abaixo do mínimo permitido, deve ser resolvido pela recursão de tratamento de underflow
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

    // Estrutura de decisão para tratamento de underflow:
    // 1: Empréstimo na direita (Se o irmão direito tiver chaves suficientes para emprestar)
    if (irmDir != NULL && irmDir->nroChaves > MIN_CHAVES) {
        int chavesTotal = 1 + irmDir->nroChaves;
        int chavesEsq = chavesTotal / 2; // Garante a regra: "nó mais à esquerda deverá conter uma chave a mais"
        int chavesDir = chavesTotal - chavesEsq - 1;

        int bufC[4], bufPr[4], bufP[5]; // buffers temporários para reorganizar as chaves e ponteiros durante o empréstimo
        bufC[0] = pai->C[indicePonteiroFilho]; bufPr[0] = pai->Pr[indicePonteiroFilho]; bufP[0] = atual->P[0];

        // Preenche os buffers com as chaves e ponteiros do irmão direito, deslocando-os para a direita para abrir espaço para a chave promovida do pai
        for(int i = 0; i < irmDir->nroChaves; i++) {
            bufC[i+1] = irmDir->C[i]; 
            bufPr[i+1] = irmDir->Pr[i]; 
            bufP[i+1] = irmDir->P[i];
        }

        bufP[irmDir->nroChaves + 1] = irmDir->P[irmDir->nroChaves];

        // Reorganiza o nó atual (em underflow), o irmão direito e o pai de acordo com a redistribuição das chaves
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
    
        // O ponteiro mais à direita do irmão direito é atualizado para o que estava originalmente no irmão direito, deslocado para a direita
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

    // 2: Empréstimo na esquerda (Se o irmão esquerdo tiver chaves suficientes para emprestar)
    if (irmEsq != NULL && irmEsq->nroChaves > MIN_CHAVES) {
        int chavesTotal = irmEsq->nroChaves + 1;
        int chavesEsq = chavesTotal / 2;
        int chavesDir = chavesTotal - chavesEsq - 1;

        int bufC[4], bufPr[4], bufP[5];
        for(int i = 0; i < irmEsq->nroChaves; i++) { // Preenche os buffers com as chaves e ponteiros do irmão esquerdo, mantendo a ordem original
            bufC[i] = irmEsq->C[i]; 
            bufPr[i] = irmEsq->Pr[i]; 
            bufP[i] = irmEsq->P[i];
        }

        // O ponteiro mais à direita do irmão esquerdo é movido para o nó em underflow, e a chave do pai que separa os dois irmãos é promovida para o nó em underflow
        bufP[irmEsq->nroChaves] = irmEsq->P[irmEsq->nroChaves];
        bufC[irmEsq->nroChaves] = pai->C[indicePonteiroFilho - 1]; bufPr[irmEsq->nroChaves] = pai->Pr[indicePonteiroFilho - 1];
        bufP[irmEsq->nroChaves + 1] = atual->P[0];

        // Reorganiza o irmão esquerdo, o nó em underflow e o pai de acordo com a redistribuição das chaves
        irmEsq->nroChaves = chavesEsq;
        for(int i = 0; i < chavesEsq; i++) { 
            irmEsq->C[i] = bufC[i]; 
            irmEsq->Pr[i] = bufPr[i]; 
            irmEsq->P[i] = bufP[i]; 
        }

        // O ponteiro mais à direita do irmão esquerdo é atualizado para o que estava originalmente no irmão esquerdo, deslocado para a direita
        irmEsq->P[chavesEsq] = bufP[chavesEsq];
        for(int i = chavesEsq; i < 3; i++) { 
            irmEsq->C[i] = -1; 
            irmEsq->Pr[i] = -1; 
            irmEsq->P[i+1] = -1; 
        }

        pai->C[indicePonteiroFilho - 1] = bufC[chavesEsq]; pai->Pr[indicePonteiroFilho - 1] = bufPr[chavesEsq];

        // O nó em underflow recebe as chaves restantes após a chave promovida para o pai
        atual->nroChaves = chavesDir;
        for(int i = 0; i < chavesDir; i++) {
            atual->C[i] = bufC[chavesEsq + 1 + i]; 
            atual->Pr[i] = bufPr[chavesEsq + 1 + i]; 
            atual->P[i] = bufP[chavesEsq + 1 + i];
        }
        atual->P[chavesDir] = bufP[chavesEsq + 1 + chavesDir];

        // O ponteiro mais à direita do nó em underflow é atualizado para o que estava originalmente no irmão esquerdo, deslocado para a direita
        fseek(fileIndice, BTREE_NO_INICIO(rrnPai), SEEK_SET); escreverNo(fileIndice, pai);
        fseek(fileIndice, BTREE_NO_INICIO(rrnAtual), SEEK_SET); escreverNo(fileIndice, atual);
        fseek(fileIndice, BTREE_NO_INICIO(rrnIrmEsq), SEEK_SET); escreverNo(fileIndice, irmEsq);

        free(atual); 
        free(irmEsq); 
        if(irmDir) free(irmDir); 
        
        return;
    }

    // 3: Merge na esquerda (O Enunciado exige tentar Esquerda primeiro na Concatenação)
    if (irmEsq != NULL)
        fazerMerge(fileIndice, irmEsq, atual, pai, rrnIrmEsq, rrnAtual, rrnPai, indicePonteiroFilho - 1, nroNos);

    // 4: Merge na direita (Se não for possível na esquerda)
    else if (irmDir != NULL)
        fazerMerge(fileIndice, atual, irmDir, pai, rrnAtual, rrnIrmDir, rrnPai, indicePonteiroFilho, nroNos);
    
    free(atual); 
    if(irmEsq) free(irmEsq); 
    if(irmDir) free(irmDir);
}

void fazerMerge(FILE *fileIndice, No *esq, No *dir, No *pai, int rrnEsq, int rrnDir, int rrnPai, int indiceChavePai, int *nroNos) {
    // A chave do pai que separa os dois nós é movida para o nó da esquerda, e as chaves do nó da direita são anexadas à direita do nó da esquerda
    esq->C[esq->nroChaves] = pai->C[indiceChavePai];
    esq->Pr[esq->nroChaves] = pai->Pr[indiceChavePai];
    esq->nroChaves++;

    // Anexa as chaves do nó da direita à direita do nó da esquerda, mantendo a ordem original
    for (int i = 0; i < dir->nroChaves; i++) {
        esq->C[esq->nroChaves + i] = dir->C[i];
        esq->Pr[esq->nroChaves + i] = dir->Pr[i];
        esq->P[esq->nroChaves + i] = dir->P[i];
    }
    esq->P[esq->nroChaves + dir->nroChaves] = dir->P[dir->nroChaves];
    esq->nroChaves += dir->nroChaves;

    // Após o merge, a chave que separava os dois nós no pai é removida, e os ponteiros são ajustados para fechar o espaço deixado pelo nó da direita que foi fundido
    for (int i = indiceChavePai; i < pai->nroChaves - 1; i++) {
        pai->C[i] = pai->C[i+1];
        pai->Pr[i] = pai->Pr[i+1];
        pai->P[i+1] = pai->P[i+2];
    }

    // O número de chaves do pai é decrementado, e os campos restantes são limpos para manter a consistência dos dados
    pai->nroChaves--;
    pai->C[pai->nroChaves] = -1;
    pai->Pr[pai->nroChaves] = -1;
    pai->P[pai->nroChaves + 1] = -1;

    // Escreve as alterações de volta no arquivo para o pai e o nó resultante do merge (nó da esquerda)
    fseek(fileIndice, BTREE_NO_INICIO(rrnEsq), SEEK_SET); escreverNo(fileIndice, esq);
    fseek(fileIndice, BTREE_NO_INICIO(rrnPai), SEEK_SET); escreverNo(fileIndice, pai);
    
    apagarNo(fileIndice, rrnDir, nroNos); // O nó da direita é logicamente apagado, mas seu espaço não é reutilizado para novas inserções, seguindo a política de alocação de nós do projeto
}

bool deleteWhereIndexado(FILE *fileDados, FILE *fileIndice, CampoValor *pares, int mPares, int *nroNos) {
    if (!fileDados || !fileIndice) return false;

    char statusDados, statusIndice;
    int topoDados;

    // Dado o status na main, apenas realiza a leitura do topo o qual será usado para a lógica de remoção
    fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET);
    if (fread(&topoDados, sizeof(int), 1, fileDados) != 1) return false;

    bool ok = true;
    int idxCodEstacao = encontrarIndexCampo(pares, mPares, "codEstacao");

    if (idxCodEstacao != -1 && !valorEhNulo(pares[idxCodEstacao].valor)) {
        // Busca O(log n) por índices da Árvore-B, seguida de validação completa dos filtros no registro encontrado
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
                    
                    // Atualiza o topo da lista de removidos no arquivo de dados para apontar para o registro recém-removido, que agora é o primeiro da lista
                    topoDados = rrnAlvo;
                    fseek(fileDados, 1, SEEK_SET); 
                    fwrite(&topoDados, sizeof(int), 1, fileDados);
                        
                    removerChaveIndice(fileIndice, chaveBuscada, nroNos);
                }
            }
        }
    } else {
        // Busca sequencial O(n), lendo diretamente no disco
        int rrn = -1;
        while (true) {
            // Passamos NULL para o índice, forçando o "full-scan" no selectWhere
            rrn = selectWhere(fileDados, NULL, pares, mPares, rrn + 1, true, true);
            if (rrn < 0) break;

            long inicioRegistro = (long)TAM_CABECALHO + (long)rrn * (long)TAM_REG;
            
            // Lê a chave (codEstacao) da estrutura do registro antes de marcar como removido e pula caso flag removido (1 byte) + proxRRN (4 bytes)
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
        if (ok) { // Após a remoção lógica de todos os registros que batem com os filtros, atualiza o topo da lista de removidos no arquivo de dados para apontar para o primeiro registro logicamente removido encontrado durante o processo
            fseek(fileDados, DADOS_OFF_TOPO, SEEK_SET);
            fwrite(&topoDados, sizeof(int), 1, fileDados);
        }
    }

    // Atualização dos dados persistentes
    recalcularContadores(fileDados);

    return ok;
}