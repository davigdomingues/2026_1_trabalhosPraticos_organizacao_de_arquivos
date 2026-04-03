dev:
	gcc -o programaTrab src/*.c c-hashmap/map.c
	./programaTrab

debug:
	gcc -g -o programaTrab src/*.c c-hashmap/map.c
	gdb ./programaTrab

all:
	gcc -o programaTrab src/*.c c-hashmap/map.c

run:
	./programaTrab