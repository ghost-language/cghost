CC=clang
CFLAGS=-I

ghost: chunk.c debug.c main.c memory.c value.c vm.c
	$(CC) -o ghost chunk.c debug.c main.c memory.c value.c vm.c

clean:
	rm ghost