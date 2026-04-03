#include "../headers/registro.h"
#include "../headers/cabecalho.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

Registro inicializarReg(){
    Registro reg;
    reg.removido = '0';
    reg.proximo = -1;
    reg.codEstacao = -1;
    reg.codLinha = -1;
    reg.codProxEstacao = -1;
    reg.distProxEstacao = -1;
    reg.codLinhaIntegra = -1;
    reg.codEstIntegra = -1;
    reg.tamNomeEstacao = 0;
    reg.nomeEstacao = "";
    reg.tamNomeLinha = 0;
    reg.nomeLinha = "";

    return reg;
}

void escreverReg(FILE *file, Registro *reg){
    fwrite(&reg->removido, sizeof(char), 1, file);
    fwrite(&reg->proximo, sizeof(int), 1, file);
    fwrite(&reg->codEstacao, sizeof(int), 1, file);
    fwrite(&reg->codLinha, sizeof(int), 1, file);
    fwrite(&reg->codProxEstacao, sizeof(int), 1, file);
    fwrite(&reg->distProxEstacao, sizeof(int), 1, file);
    fwrite(&reg->codLinhaIntegra, sizeof(int), 1, file);
    fwrite(&reg->codEstIntegra, sizeof(int), 1, file);
    fwrite(&reg->tamNomeEstacao, sizeof(int), 1, file);
    fwrite(reg->nomeEstacao, sizeof(char), reg->tamNomeEstacao, file);
    fwrite(&reg->tamNomeLinha, sizeof(int), 1, file);
    fwrite(reg->nomeLinha, sizeof(char), reg->tamNomeLinha, file);

    //subtrai do tamanho total o tamanho dos campos fixos e o tamanho dos campos variáveis
    int tamRestante = TAM_LIVRE_REG(reg->tamNomeEstacao, reg->tamNomeLinha);
    char filler[tamRestante];
    //preenche o restante do registro com '$'
    memset(filler, '$', tamRestante);
    fwrite(filler, sizeof(char), tamRestante, file);
}

void printReg(Registro *reg){
    //campos não nulos
    printf("%d ", reg->codEstacao);
    printf("%s ", reg->nomeEstacao);

    //campos possívelmente nulos
    if(reg->codLinha != -1) printf("%d ", reg->codLinha);
    else printf("%s ", "NULO");

    if(reg->tamNomeLinha != 0) printf("%s ", reg->nomeLinha);
    else printf("%s ", "NULO");

    if(reg->codProxEstacao != -1) printf("%d ", reg->codProxEstacao);
    else printf("%s ", "NULO");

    if(reg->distProxEstacao != -1) printf("%d ", reg->distProxEstacao);
    else printf("%s ", "NULO");

    if(reg->codLinhaIntegra != -1) printf("%d ", reg->codLinhaIntegra);
    else printf("%s ", "NULO");

    if(reg->codEstIntegra != -1) printf("%d", reg->codEstIntegra);
    else printf("%s", "NULO");

    printf("\n");
}

// retorna true se o nome da estação já existe em algum registro ativo do arquivo
bool nomeEstacaoJaExiste(FILE *file, const char *nomeEstacao, int tamNomeEstacao) {
    if (!file || !nomeEstacao || tamNomeEstacao <= 0) return false;

    long posOriginal = ftell(file); // salva a posição original do ponteiro de arquivo para poder voltar depois
    fseek(file, TAM_CABECALHO, SEEK_SET); // pula o cabeçalho

    // varre o arquivo lendo os registros um por um, procurando por um registro ativo que tenha o mesmo nome de estação
    while (1) {
        char removido;
        if (fread(&removido, sizeof(char), 1, file) != 1) break; // EOF

        if (removido == '1') { // registro removido: pula o resto do registro sem ler
            fseek(file, TAM_REG - 1, SEEK_CUR);
            continue;
        }

        // pula proximo + codEstacao + codLinha + codProxEstacao + dist + codLinhaIntegra + codEstIntegra
        fseek(file, 7 * (long)sizeof(int), SEEK_CUR);

        int tamLido = 0;
        if (fread(&tamLido, sizeof(int), 1, file) != 1) break; // EOF

        if (tamLido > 0) {
            if (tamLido == tamNomeEstacao) { // só lê o nome da estação se o tamanho for igual, para evitar ler strings desnecessárias
                char buf[256];
                if (tamLido < (int)sizeof(buf)) { // se o nome da estação couber no buffer, lê diretamente para ele, evitando alocações desnecessárias
                    if (fread(buf, sizeof(char), (size_t)tamLido, file) != (size_t)tamLido) break;
                    if (memcmp(buf, nomeEstacao, (size_t)tamLido) == 0) { // compara o nome lido com o nome da estação procurada
                        fseek(file, posOriginal, SEEK_SET);
                        return true;
                    }
                } else { // se o nome da estação for muito grande para o buffer, lê para uma string alocada dinamicamente
                    char *tmp = (char*) malloc((size_t)tamLido);
                    if (!tmp) break;
                    if (fread(tmp, sizeof(char), (size_t)tamLido, file) != (size_t)tamLido) {
                        free(tmp);
                        break;
                    }
                    bool igual = (memcmp(tmp, nomeEstacao, (size_t)tamLido) == 0); // compara o nome lido com o nome da estação procurada
                    free(tmp);
                    if (igual) { // encontrou um registro ativo com o mesmo nome de estação
                        fseek(file, posOriginal, SEEK_SET);
                        return true;
                    }
                }

            } else {
                fseek(file, tamLido, SEEK_CUR); // pula o nome da estação se o tamanho não for igual
            }
        }

        int tamNomeLinha = 0;
        if (fread(&tamNomeLinha, sizeof(int), 1, file) != 1) break; // EOF
        if (tamNomeLinha > 0) fseek(file, tamNomeLinha, SEEK_CUR); // pula o nome da linha

        int tamRestante = TAM_LIVRE_REG(tamLido, tamNomeLinha); // calcula o tamanho do lixo ($) a ser pulado para ir para o próximo registro
        if (tamRestante > 0) fseek(file, tamRestante, SEEK_CUR);
    }

    fseek(file, posOriginal, SEEK_SET); // devolve o ponteiro de arquivo para a posição original
    return false;
}