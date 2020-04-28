#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "vendor/linenoise.h"

#define VERSION "dev-master"

static void repl() {
    char *line;

    puts("Ghost (" ANSI_COLOR_CYAN VERSION ANSI_COLOR_RESET ")");
    puts("Press Ctrl + C to exit\n");

    linenoiseHistoryLoad("ghost_history.txt");

    while ((line = linenoise(">>> ")) != NULL)
    {
        char *fullLine = malloc(sizeof(char) * (strlen(line) + 1));
        snprintf(fullLine, strlen(line) + 1, "%s", line);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave("ghost_history.txt");

        interpret(fullLine);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, ANSI_COLOR_RED "Could not open file \"%s\"." ANSI_COLOR_RESET "\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);

    if (buffer == NULL) {
        fprintf(stderr, ANSI_COLOR_RED "Not enough memory to read \"%s\"." ANSI_COLOR_RESET "\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    if (bytesRead < fileSize) {
        fprintf(stderr, ANSI_COLOR_RED "Could not read file \"%s\"." ANSI_COLOR_RESET "\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);

    return buffer;
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    initVM();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: ghost [path]\n");
        exit(64);
    }

    freeVM();

    return 0;
}