CC=clang
CFLAGS=-I

ghost: chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c
	$(CC) -o ghost chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c

clean:
	rm ghost