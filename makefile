# detecção do SO para escolha da ferramenta de detecção de vazamento
ifeq ($(OS),Windows_NT)
    DETECK_OS := Windows
else
    DETECK_OS := $(shell uname -s)
endif

dev:
	gcc -o programaTrab src/*.c src/dados/*.c src/indice/*.c src/operacoes/*.c c-hashmap/map.c

debug:
	gcc -g -o programaTrab src/*.c src/dados/*.c src/indice/*.c src/operacoes/*.c c-hashmap/map.c
	gdb ./programaTrab

all:
	gcc -o programaTrab src/*.c src/dados/*.c src/indice/*.c src/operacoes/*.c c-hashmap/map.c

run:
	./programaTrab

# comando genérico de detecção de vazamento de memória, adaptado para cada SO
leak-check:
ifeq ($(DETECK_OS),Darwin)
	gcc -g -o programaTrab src/*.c src/dados/*.c src/indice/*.c src/operacoes/*.c c-hashmap/map.c
	leaks --atExit -- ./programaTrab > relatorio_leak.txt 2>&1

else ifeq ($(DETECK_OS),Linux)
	gcc -g -o programaTrab src/*.c src/dados/*.c src/indice/*.c src/operacoes/*.c c-hashmap/map.c
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./programaTrab > relatorio_leak.txt 2>&1

else
	gcc -g -fsanitize=address -fno-omit-frame-pointer -o programaTrab src/*.c src/dados/*.c src/indice/*.c src/operacoes/*.c c-hashmap/map.c
	./programaTrab > relatorio_leak.txt 2>&1
endif