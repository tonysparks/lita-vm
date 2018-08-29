#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "vm.h"
#include "common.h"

static void vmError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    exit(2);
}


Ram* ramInit(size_t size) {
    void* mem = litaMalloc(sizeof(Ram) + size);
    Ram* ram = (Ram*) mem;
    ram->size = size;
    ram->mem = mem + sizeof(Ram);

    return ram;
}


void ramFree(Ram* ram) {
    if(ram) {
        litaFree(ram);
    }
}

#define CHECK_RANGE(ram, startAddress, endAddress) do { if( (startAddress) < 0 || ((startAddress) + (endAddress)) >= (ram)->size) vmError("Access violation error at address '0x%x' to '0x%x' \n", (startAddress), ((startAddress) + (endAddress)) ); } while(0)

void ramStoreString(Ram* ram, size_t address, const char* value, size_t len) {
    CHECK_RANGE(ram, address, len);

    memcpy(ram->mem + address, value, len);
    ram->mem[address + len] = 0;
}

void ramStoreBytes(Ram* ram, size_t address, const char* value, size_t len) {
    CHECK_RANGE(ram, address, len);

    memcpy(ram->mem + address, value, len);
}

void ramStoreInt32(Ram* ram, size_t address, int32_t value) {
    CHECK_RANGE(ram, address, sizeof(value));

    memcpy(ram->mem + address, &value, sizeof(value));
}

void ramStoreFloat(Ram* ram, size_t address, float value) {
    CHECK_RANGE(ram, address, sizeof(value));

    memcpy(ram->mem + address, &value, sizeof(value));
}

void ramStoreInt8(Ram* ram, size_t address, int8_t value) {
    CHECK_RANGE(ram, address, sizeof(value));

    memcpy(ram->mem + address, &value, sizeof(value));
}




size_t ramReadBytes(Ram* ram, size_t address, char* value, size_t len) {
    CHECK_RANGE(ram, address, len);

    memcpy(value, ram->mem + address, len);

    return len;
}

int32_t ramReadInt32(Ram* ram, size_t address) {
    CHECK_RANGE(ram, address, sizeof(int32_t));

    int32_t result = 0;
    memcpy(&result, ram->mem + address, sizeof(int32_t));

    return result;
}

float ramReadFloat(Ram* ram, size_t address) {
    CHECK_RANGE(ram, address, sizeof(float));
    
    float result = 0;
    memcpy(&result, ram->mem + address, sizeof(float));

    return result;
}

int8_t ramReadInt8(Ram* ram, size_t address) {
    CHECK_RANGE(ram, address, sizeof(int8_t));
    
    int8_t result = 0;
    memcpy(&result, ram->mem + address, sizeof(int8_t));

    return result;
}