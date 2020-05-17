#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "utilities.h"
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