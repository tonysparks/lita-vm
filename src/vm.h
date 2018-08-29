#ifndef LITA_VM_H
#define LITA_VM_H

#include <stdint.h>

typedef struct Ram {
    size_t size;
    char*  mem;
} Ram;

Ram* ramInit(size_t size);
void ramFree(Ram* ram);

void ramStoreString(Ram* ram, size_t address, const char* value, size_t len);
void ramStoreBytes(Ram* ram, size_t address, const char* value, size_t len);
void ramStoreInt32(Ram* ram, size_t address, int32_t value);
void ramStoreFloat(Ram* ram, size_t address, float value);
void ramStoreInt8(Ram* ram, size_t address, int8_t value);

size_t ramReadBytes(Ram* ram, size_t address, char* value, size_t len);
int32_t ramReadInt32(Ram* ram, size_t address);
float ramReadFloat(Ram* ram, size_t address);
int8_t ramReadInt8(Ram* ram, size_t address);

typedef struct Vm {
    size_t stackSize;

} Vm;

#endif