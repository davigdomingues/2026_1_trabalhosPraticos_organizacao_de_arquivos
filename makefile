dev:
	gcc -o programaTrab *.c c-hashmap/map.c
	./programaTrab

debug:
	gcc -g -o programaTrab *.c c-hashmap/map.c
	gdb ./programaTrab

all:
	gcc -o programaTrab *.c c-hashmap/map.c

run:
	./programaTrab