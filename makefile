CC=clang
CFLAGS=-I

ghost: src/chunk.c src/compiler.c src/debug.c src/main.c src/memory.c src/scanner.c src/value.c src/vm.c
	$(CC) -o dist/ghost src/chunk.c src/compiler.c src/debug.c src/main.c src/memory.c src/scanner.c src/value.c src/vm.c

clean:
	rm dist/ghost