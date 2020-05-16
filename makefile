CC=clang
CFLAGS=-I

ghost: src/chunk.c src/compiler.c src/debug.c src/main.c src/memory.c src/native.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c src/modules/modules.c src/modules/math.c src/vendor/linenoise.c
	@ $(CC) -o ghost src/chunk.c src/compiler.c src/debug.c src/main.c src/memory.c src/native.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c src/modules/modules.c src/modules/math.c src/vendor/linenoise.c

clean:
	@ rm ghost