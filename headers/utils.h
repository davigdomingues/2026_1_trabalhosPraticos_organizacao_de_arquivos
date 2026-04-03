#include "../c-hashmap/map.h"
#include "../headers/registro.h"
#include "../headers/operacoes.h"

int freeMapKeys(const void* key, size_t ksize, uintptr_t value, void* usr);
bool verificarMatchInt(int index, char *valorQuery, int valorReg);
bool verificarMatchStr(int index, char *valorQuery, char *valorReg);
bool valorEhNulo(const char *valor);
bool registroMatchParBusca(const Registro *reg, const char *nomeEstacao, const char *nomeLinha, const CampoValor *par);
int encontrarIndexCampo(CampoValor *pares, int mPares, const char *campo);