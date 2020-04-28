CC=clang
CFLAGS=-I
BUILD_DIR := build

ghost: src/chunk.c src/compiler.c src/debug.c src/main.c src/memory.c src/native.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c src/modules/math.c
	@ mkdir $(BUILD_DIR)
	@ $(CC) -o $(BUILD_DIR)/ghost src/chunk.c src/compiler.c src/debug.c src/main.c src/memory.c src/native.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c src/modules/math.c
	@ cp $(BUILD_DIR)/ghost ghost

clean:
	@ rm -rf $(BUILD_DIR)
	@ rm ghost