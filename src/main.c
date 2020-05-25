#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "include/ghost.h"
#include "utilities.h"
#include "vm.h"
#include "vendor/linenoise.h"

#define VERSION "dev-master"

static void repl(GhostVM *vm) {
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

        ghostInterpret(vm, fullLine);
    }
}

static void *reallocate(void *memory, size_t oldSize, size_t newSize)
{
    return realloc(memory, newSize);
}

static void runFile(GhostVM *vm, const char* path) {
    char* source = readFile(path);
    InterpretResult result = ghostInterpret(vm, source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    GhostVM *vm = ghostNewVM(reallocate);

    if (argc == 1) {
        repl(vm);
    } else if (argc == 2) {
        runFile(vm, argv[1]);
    } else {
        fprintf(stderr, "Usage: ghost [path]\n");
        exit(64);
    }

    ghostFreeVM(vm);

    return 0;
}