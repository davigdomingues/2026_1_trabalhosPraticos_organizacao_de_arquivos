# detecção do SO para escolha da ferramenta de detecção de vazamento
ifeq ($(OS),Windows_NT)
    DETECK_OS := Windows
else
    DETECK_OS := $(shell uname -s)
endif

# Variável para agrupar as fontes e evitar repetição de código
SRC_FILES := src/*.c src/dados/*.c src/indice/*.c src/operacoes/*.c c-hashmap/map.c

all:
	gcc -o programaTrab $(SRC_FILES)

dev:
	gcc -o programaTrab $(SRC_FILES)
	./programaTrab

run:
	./programaTrab

# GDB atualizado para também compilar com AddressSanitizer (ajuda muito no debug)
debug:
	gcc -g -fsanitize=address -fno-omit-frame-pointer -o programaTrab $(SRC_FILES)
	gdb ./programaTrab

# Comando genérico de checagem profunda de memória adaptado por SO
leak-check:
ifeq ($(DETECK_OS),Darwin)
	# macOS: Primeiro compila e roda APENAS com ASan para ver erros de acesso
	@echo "=== Executando AddressSanitizer (Erros de Acesso) ===" > relatorio_leak.txt
	gcc -g -fsanitize=address -fno-omit-frame-pointer -o programaTrab_asan $(SRC_FILES)
	-./programaTrab_asan >> relatorio_leak.txt 2>&1
	rm -f programaTrab_asan

	# macOS: Depois compila normal e roda com o leaks para ver vazamentos
	@echo "\n=== Executando Leaks (Vazamentos de Memória) ===" >> relatorio_leak.txt
	gcc -g -o programaTrab $(SRC_FILES)
	export MALLOC_STACK_LOGGING=1; leaks --atExit -- ./programaTrab >> relatorio_leak.txt 2>&1

else ifeq ($(DETECK_OS),Linux)
	# Linux/WSL: Valgrind nativo (não use ASan aqui, pois eles entram em conflito)
	gcc -g -o programaTrab $(SRC_FILES)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./programaTrab > relatorio_leak.txt 2>&1

else
	# Outros (Windows nativo / MinGW): Usa apenas o ASan do GCC
	gcc -g -fsanitize=address -fno-omit-frame-pointer -o programaTrab $(SRC_FILES)
	./programaTrab > relatorio_leak.txt 2>&1
endif