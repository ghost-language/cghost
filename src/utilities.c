#include <stdio.h>
#include <stdlib.h>

#include "colors.h"

// Reads the contents of the file at [path] and returns
// it as a heap allocated string.
//
// Returns NULL if the path could not be found. Exits if
// it was found but could not be read.
char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, ANSI_COLOR_RED "Could not open file \"%s\"." ANSI_COLOR_RESET "\n", path);
        exit(74);
    }

    // Find out how big the file is
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Allocate a buffer for it
    char* buffer = (char*)malloc(fileSize + 1);

    if (buffer == NULL) {
        fprintf(stderr, ANSI_COLOR_RED "Not enough memory to read \"%s\"." ANSI_COLOR_RESET "\n", path);
        exit(74);
    }

    // Read the entire file
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    if (bytesRead < fileSize) {
        fprintf(stderr, ANSI_COLOR_RED "Could not read file \"%s\"." ANSI_COLOR_RESET "\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);

    return buffer;
}