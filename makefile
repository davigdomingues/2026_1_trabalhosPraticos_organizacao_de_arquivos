dev:
	gcc -o programaTrab src/*.c src/dados/*.c src/indice/*.c c-hashmap/map.c
	./programaTrab

debug:
	gcc -g -o programaTrab src/*.c src/dados/*.c src/indice/*.c c-hashmap/map.c
	gdb ./programaTrab

all:
	gcc -o programaTrab src/*.c src/dados/*.c src/indice/*.c c-hashmap/map.c

run:
	./programaTrab