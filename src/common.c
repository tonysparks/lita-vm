// standard includes
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#ifndef _WINDOWS    
    #define strcasecmp _stricmp
#endif

void* litaMalloc(size_t size) {
    return malloc(size);
}

void* litaRealloc(void* ptr, size_t newSize) {
    return realloc(ptr, newSize);
}


void  litaFree(void* mem) {
    return free(mem);
}



char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(1);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)litaMalloc(fileSize + 1);

    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(1);
    }
  

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(1);
    }
  
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

int   strStartsWith(const char* prefix, const char* str) {
    size_t lenpre = strlen(prefix),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(prefix, str, lenpre) == 0;
}

int   strCmpIgnoreCase(const char* a, const char* b) {
    return strcasecmp(a, b);
}