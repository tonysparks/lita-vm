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
    char* mem = litaMalloc(sizeof(Ram) + size);
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


Cpu32* cpuInit() {
    Cpu32* cpu = (Cpu32*) litaMalloc(sizeof(Cpu32));
    return cpu;
}
void   cpuFree(Cpu32* cpu) {
    if(cpu) {
        litaFree(cpu);
    }
}

Vm*  vmInit(size_t stackSize, size_t ramSize) {
    Ram* ram = ramInit(ramSize);
    Cpu32* cpu = cpuInit();

    Vm* vm = (Vm*) litaMalloc(sizeof(Vm));
    vm->ram = ram;
    vm->cpu = cpu;
    vm->stackSize = stackSize;

    return vm;
}


void vmFree(Vm* vm) {
    if(vm) {
        cpuFree(vm->cpu);
        ramFree(vm->ram);
        litaFree(vm);
    }
}

ExecutionResult vmExecute(Vm* vm, Bytecode* code) {
    ExecutionResult result = {0};
    
    Cpu32* cpu = vm->cpu;
    Ram* ram = vm->ram;

    if(!code->length || !code->instrs) {
        return result;
    }

#define INSTR_AT(index) (&code->instrs[(index)])

#define SET_ARG1_INT(instr,value)         \
    do {                                  \
        if (IS_ARG1_ADDR(instr))          \
            ramStoreInt32(ram, cpu->regs[ARG1_VALUE(instr)].as.address,(value));  \
        else cpu->regs[ARG1_VALUE(instr)].as.iVal = (value);                      \
    } while(0)

#define SET_ARG1_FLOAT(instr,value)       \
    do {                                  \
        if (IS_ARG1_ADDR(instr))          \
            ramStoreFloat(ram, cpu->regs[ARG1_VALUE(instr)].as.address,(value));  \
        else cpu->regs[ARG1_VALUE(instr)].as.fVal = (value);                      \
    } while(0)

#define SET_ARG1_INT8(instr,value)       \
    do {                                 \
        if (IS_ARG1_ADDR(instr))         \
            ramStoreInt8(ram, cpu->regs[ARG1_VALUE(instr)].as.address,(value));  \
        else cpu->regs[ARG1_VALUE(instr)].as.bVal = (value);                     \
    } while(0)

#define GET_ARG1_INT(instr)                                        \
    ((IS_ARG1_ADDR(instr)) ?                                       \
        ramReadInt32(ram, cpu->regs[ARG1_VALUE(instr)].as.address) \
        : cpu->regs[ARG1_VALUE(instr)].as.iVal)

#define GET_ARG2_INT(instr)                                        \
    ((IS_ARG2_ADDR(instr)) ?                                       \
        ramReadInt32(ram, cpu->regs[ARG2_VALUE(instr)].as.address) \
        : cpu->regs[ARG2_VALUE(instr)].as.iVal)

#define GET_ARG2_FLOAT(instr)                                      \
    ((IS_ARG2_ADDR(instr)) ?                                       \
        ramReadFloat(ram, cpu->regs[ARG2_VALUE(instr)].as.address) \
        : cpu->regs[ARG2_VALUE(instr)].as.fVal)

#define GET_ARG2_INT8(instr)                                       \
    ((IS_ARG2_ADDR(instr)) ?                                       \
        ramReadInt8(ram, cpu->regs[ARG2_VALUE(instr)].as.address)  \
        : cpu->regs[ARG2_VALUE(instr)].as.bVal)

    Instruction* pc = code->instrs;
    Instruction* end = INSTR_AT(code->length - 1);
        
    printf("Executing code\n");
    
    while(pc <= end) {
        cpu->pc.as.address = pc - code->instrs; // TODO should address be a size_t or void*???

        Instruction instr = *pc++;
        
        printf("I: %d\n", instr);
        printf("Opcode: '%s' \n", OpcodeStr[instr]);

        switch(instr) {
            case NOOP: {
                break;
            }
            case JMP: {
                pc = INSTR_AT(ARG_JMP_VALUE(instr));
                break;
            }
            case CALL: {
                cpu->r.as.address = pc - code->instrs;
                pc = INSTR_AT(ARG_JMP_VALUE(instr));
                break;
            }
            case RET: {
                pc = INSTR_AT(cpu->r.as.address);
                break;
            }
            case MOVI: {
                SET_ARG1_INT(instr, GET_ARG2_INT(instr));
                break;
            }
            case MOVF: {
                SET_ARG1_FLOAT(instr, GET_ARG2_FLOAT(instr));
                break;
            }
            case MOVB: {
                SET_ARG1_INT8(instr, GET_ARG2_INT8(instr));
                break;
            }
        }
    }

#undef INSTR_AT    
    return result;
}
