#include "../../headers/indice/btree_no.h"
#include "../../headers/indice/btree_cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static void limparNo(No *no) {
    // inicializa os campos do nó com valores padrão, indicando que estão vazios ou nulos
    no->removido = '0';
    no->proximo = -1;
    no->tipoNo = NO_NAO_INICIALIZADO;
    no->nroChaves = 0;

    // inicializa os arrays de chaves, ponteiros para registros e ponteiros para subárvores (vazios)
    for (int i = 0; i < NRO_MAX_CHAVES; i++) {
        no->C[i] = -1;
        no->Pr[i] = -1;
        no->P[i] = -1;
    }
    no->P[NRO_MAX_CHAVES] = -1;
}

No *inicializarNo(){
    No *no = (No*) malloc(sizeof(No));
    
    limparNo(no); // garante que o nó começa com um estado limpo e consistente, evitando lixo de memória
    return no;
}

void lerNo(FILE *fileIndice, No *no){
    fread(&no->removido, sizeof(char), 1, fileIndice); // lê a flag de removido para verificar se o nó está logicamente excluído
    fread(&no->proximo, sizeof(int), 1, fileIndice); // lê o campo de encadeamento para nós removidos, caso seja necessário reaproveitar esse nó no futuro
    fread(&no->tipoNo, sizeof(int), 1, fileIndice); // lê o tipo do nó (folha ou interna) para determinar como interpretar os campos seguintes
    fread(&no->nroChaves, sizeof(int), 1, fileIndice); // lê o número de chaves atualmente armazenadas no nó, para saber quantos campos de chaves e ponteiros são válidos
    fread(&no->C[0], sizeof(int), 1, fileIndice); // lê a primeira chave do nó
    fread(&no->Pr[0], sizeof(int), 1, fileIndice); // lê o ponteiro para o registro associado à primeira chave, se for um nó folha
    fread(&no->C[1], sizeof(int), 1, fileIndice); // lê a segunda chave do nó
    fread(&no->Pr[1], sizeof(int), 1, fileIndice); // lê o ponteiro para o registro associado à segunda chave, se for um nó folha
    fread(&no->C[2], sizeof(int), 1, fileIndice); // lê a terceira chave do nó
    fread(&no->Pr[2], sizeof(int), 1, fileIndice); // lê o ponteiro para o registro associado à terceira chave, se for um nó folha
    fread(&no->P, sizeof(int) * (NRO_MAX_CHAVES+1), 1, fileIndice); // lê os ponteiros para as subárvores (filhos) do nó, se for um nó interno, ou mantém como -1 se for um nó folha
}

No *criarNo(FILE *fileIndice, int *novoRRN){
    // verifica se há nós removidos disponíveis para reaproveitamento, lendo o topo da pilha de nós excluídos no cabeçalho do arquivo
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);

    // lê o RRN do topo da pilha de nós removidos, que indica o próximo nó disponível para reutilização
    int topo;
    fread(&topo, sizeof(int), 1, fileIndice);

    // lê o RRN do próximo nó a ser criado, que é mantido no cabeçalho do arquivo para garantir que cada novo nó receba um RRN único e sequencial
    int proxRRN;
    fread(&proxRRN, sizeof(int), 1, fileIndice);

    if(topo == -1){ // não há nós removidos para reaproveitar, então o novo nó será criado no final do arquivo usando o próximo RRN disponível
        *novoRRN = proxRRN;
        proxRRN++;

        // atualiza o próximo RRN disponível no cabeçalho do arquivo para o próximo nó a ser criado
        fseek(fileIndice, BTREE_OFF_PROXRRN, SEEK_SET);
        fwrite(&proxRRN, sizeof(int), 1, fileIndice);
    } else {
        //reaproveita um nó removido logicamente
        *novoRRN = topo;
        int inicioNoRemovido = BTREE_NO_INICIO(topo);

        //lê o novo topo
        int novoTopo;
        fseek(fileIndice, inicioNoRemovido+1, SEEK_SET); //+1 para pular a flag de removido
        fread(&novoTopo, sizeof(int), 1, fileIndice);
        
        //atualiza o topo da pilha de removidos no cabeçalho
        fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
        fwrite(&novoTopo, sizeof(int), 1, fileIndice);
    }
    return inicializarNo();
}

bool escreverNo(FILE *fileIndice, No *no){
    fwrite(&no->removido, sizeof(char), 1, fileIndice); // escreve a flag de removido para indicar se o nó está logicamente excluído ou não
    fwrite(&no->proximo, sizeof(int), 1, fileIndice); // escreve o campo de encadeamento para nós removidos, caso seja necessário reaproveitar esse nó no futuro
    fwrite(&no->tipoNo, sizeof(int), 1, fileIndice); // escreve o tipo do nó (folha ou interna) para saber interpretar os campos seguintes
    fwrite(&no->nroChaves, sizeof(int), 1, fileIndice); // escreve o número de chaves atualmente armazenadas no nó, para se saber quantos campos de chaves e ponteiros são válidos
    fwrite(&no->C[0], sizeof(int), 1, fileIndice); // escreve a primeira chave do nó
    fwrite(&no->Pr[0], sizeof(int), 1, fileIndice); // escreve o ponteiro para o registro associado à primeira chave, se for um nó folha
    fwrite(&no->C[1], sizeof(int), 1, fileIndice); // escreve a segunda chave do nó
    fwrite(&no->Pr[1], sizeof(int), 1, fileIndice); // escreve o ponteiro para o registro associado à segunda chave, se for um nó folha
    fwrite(&no->C[2], sizeof(int), 1, fileIndice); // escreve a terceira chave do nó
    fwrite(&no->Pr[2], sizeof(int), 1, fileIndice); // escreve o ponteiro para o registro associado à terceira chave, se for um nó folha
    fwrite(&no->P, sizeof(int) * (NRO_MAX_CHAVES+1), 1, fileIndice); // escreve os ponteiros para as subárvores (filhos) do nó, se for um nó interno, ou mantém como -1 se for um nó folha
    return true;
}

void apagarNo(FILE *fileIndice, int rrnNoParaApagar, int *nroNos) {
    int topoAtual;

    // Lê o topo atual da pilha de nós removidos para encadear o nó que está sendo apagado
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
    fread(&topoAtual, sizeof(int), 1, fileIndice);

    // Altera apenas o removido e o encadeamento (mantém os bytes antigos intactos)
    fseek(fileIndice, BTREE_NO_INICIO(rrnNoParaApagar), SEEK_SET);
    char removido = '1';
    fwrite(&removido, sizeof(char), 1, fileIndice);
    fwrite(&topoAtual, sizeof(int), 1, fileIndice);

    // Atualiza o topo da pilha de nós excluídos
    fseek(fileIndice, BTREE_OFF_TOPO, SEEK_SET);
    fwrite(&rrnNoParaApagar, sizeof(int), 1, fileIndice);

    if (nroNos != NULL) (*nroNos)--; // Decrementa o contador de nós da Árvore-B, apenas em memória, sem alterar o arquivo
}