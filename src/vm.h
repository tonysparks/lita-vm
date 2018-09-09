#ifndef LITA_VM_H
#define LITA_VM_H

#include <stdint.h>
#include "bytecode.h"

typedef struct Ram {
    size_t size;
    char*  mem;
} Ram;

Ram* ramInit(size_t size);
void ramFree(Ram* ram);

void ramStoreString(Ram* ram, Address address, const char* value, size_t len);
void ramStoreBytes(Ram* ram, Address address, const char* value, size_t len);
void ramStoreInt32(Ram* ram, Address address, int32_t value);
void ramStoreFloat(Ram* ram, Address address, float value);
void ramStoreInt8(Ram* ram, Address address, int8_t value);

size_t  ramReadBytes(Ram* ram, Address address, char* value, size_t len);
int32_t ramReadInt32(Ram* ram, Address address);
float   ramReadFloat(Ram* ram, Address address);
int8_t  ramReadInt8(Ram* ram, Address address);

#define ADDRESS_SIZE 4

typedef struct Register {
    union {
        int32_t  iVal;
        int8_t   bVal;
        float    fVal;
        Address  address;
    } as;
} Register;

typedef struct Cpu32 {
    union {
        struct {
            Register sp;
            Register pc;
            Register r;
            Register h;
            Register a;
            Register b;
            Register c;
            Register d;
            Register i;
            Register j;
            Register k;
            Register u;
        };
        struct {
            Register regs[12];
        };
    };
} Cpu32;

const char* RegisterNames[] = {
    "$sp",
    "$pc",
    "$r",
    "$h",
    "$a",
    "$b",
    "$c",
    "$d",
    "$i",
    "$j",
    "$k",
    "$u",
    NULL
};

Cpu32* cpuInit();
void   cpuFree(Cpu32* cpu);
int    cpuGetRegisterIndex(Cpu32* cpu, const char* name);

typedef struct Vm {
    size_t stackSize;
    Ram*   ram;
    Cpu32* cpu;
} Vm;

typedef enum ResultType {
    SUCCESS,
    ERROR,
} ResultType;

typedef struct ExecutionResult {
    ResultType result;
} ExecutionResult;

Vm*  vmInit(size_t stackSize, size_t ramSize);
void vmFree(Vm* vm);

ExecutionResult vmExecute(Vm* vm, Bytecode* code);

#endif