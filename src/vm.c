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

#define CHECK_RANGE(ram, startAddress, endAddress)                                  \
    do {                                                                            \
        if( (startAddress) < 0 || ((startAddress) + (endAddress)) >= (ram)->size) { \
            vmError("Access violation error at address '0x%x' to '0x%x' \n",        \
                (startAddress), ((startAddress) + (endAddress)) );                  \
            exit(42);                                                               \
        }                                                                           \
    } while(0)

void ramStoreString(Ram* ram, Address address, const char* value, size_t len) {
    CHECK_RANGE(ram, address, len);

    memcpy(ram->mem + address, value, len);
    ram->mem[address + len] = 0;
}

void ramStoreBytes(Ram* ram, Address address, const char* value, size_t len) {
    CHECK_RANGE(ram, address, len);

    memcpy(ram->mem + address, value, len);
}

void ramStoreInt32(Ram* ram, Address address, int32_t value) {
    CHECK_RANGE(ram, address, sizeof(value));

    memcpy(ram->mem + address, &value, sizeof(value));
}

void ramStoreFloat(Ram* ram, Address address, float value) {
    CHECK_RANGE(ram, address, sizeof(value));

    memcpy(ram->mem + address, &value, sizeof(value));
}

void ramStoreInt8(Ram* ram, Address address, int8_t value) {
    CHECK_RANGE(ram, address, sizeof(value));

    memcpy(ram->mem + address, &value, sizeof(value));
}




size_t ramReadBytes(Ram* ram, Address address, char* value, size_t len) {
    CHECK_RANGE(ram, address, len);

    memcpy(value, ram->mem + address, len);

    return len;
}

int32_t ramReadInt32(Ram* ram, Address address) {
    CHECK_RANGE(ram, address, sizeof(int32_t));

    int32_t result = 0;
    memcpy(&result, ram->mem + address, sizeof(int32_t));

    return result;
}

float ramReadFloat(Ram* ram, Address address) {
    CHECK_RANGE(ram, address, sizeof(float));
    
    float result = 0;
    memcpy(&result, ram->mem + address, sizeof(float));

    return result;
}

int8_t ramReadInt8(Ram* ram, Address address) {
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

int    cpuGetRegisterIndex(const char* name) {    
    if(!strcmp(name, "$sp") || !strcmp(name, "$SP")) {
        return 0;
    }
    else if(!strcmp(name, "$pc") || !strcmp(name, "$PC")) {
        return 1;
    }
    else if(!strcmp(name, "$r") || !strcmp(name, "$R")) {
        return 2;
    }
    else if(!strcmp(name, "$h") || !strcmp(name, "$H")) {
        return 3;
    }
    else if(!strcmp(name, "$a") || !strcmp(name, "$A")) {
        return 4;
    }
    else if(!strcmp(name, "$b") || !strcmp(name, "$B")) {
        return 5;
    }
    else if(!strcmp(name, "$c") || !strcmp(name, "$C")) {
        return 6;
    }
    else if(!strcmp(name, "$d") || !strcmp(name, "$D")) {
        return 7;
    }
    else if(!strcmp(name, "$i") || !strcmp(name, "$I")) {
        return 8;
    }
    else if(!strcmp(name, "$j") || !strcmp(name, "$J")) {
        return 9;
    }
    else if(!strcmp(name, "$k") || !strcmp(name, "$K")) {
        return 10;
    }
    else if(!strcmp(name, "$u") || !strcmp(name, "$U")) {
        return 11;
    }

    return -1;
}

Vm*  vmInit(VmConfig* config) {
    if(config->stackSize > config->ramSize) {
        vmError("Invalid VM configuration the stack size (%d) is greater than the RAM size (%d)",
            config->stackSize, config->ramSize);
    }

    Ram* ram = ramInit(config->ramSize);
    Cpu32* cpu = cpuInit();

    Vm* vm = (Vm*) litaMalloc(sizeof(Vm));
    vm->ram = ram;
    vm->cpu = cpu;
    vm->stackSize = config->stackSize;

    cpu->sp.as.address = config->ramSize - 1;
    return vm;
}


void vmFree(Vm* vm) {
    if(vm) {
        cpuFree(vm->cpu);
        ramFree(vm->ram);
        litaFree(vm);
    }
}

inline static int32_t getArg2Int32(Vm* vm, Bytecode* code, Instruction instr) {
    Ram* ram = vm->ram;
    Cpu32* cpu = vm->cpu;

    if(IS_ARG2_REG(instr)) {
        return (IS_ARG2_ADDR(instr)) 
            ? ramReadInt32(ram, cpu->regs[ARG2_VALUE(instr)].as.address)    
            : cpu->regs[ARG2_VALUE(instr)].as.iVal;
    }

    return IS_ARG2_IMM(instr) 
        ? ARG2_VALUE(instr)
        : ramReadInt32(ram, code->constants[ARG2_VALUE(instr)]);
}


inline static int8_t getArg2Int8(Vm* vm, Bytecode* code, Instruction instr) {
    Ram* ram = vm->ram;
    Cpu32* cpu = vm->cpu;

    if(IS_ARG2_REG(instr)) {
        return (IS_ARG2_ADDR(instr)) 
            ? ramReadInt8(ram, cpu->regs[ARG2_VALUE(instr)].as.address)    
            : cpu->regs[ARG2_VALUE(instr)].as.bVal;
    }

    return IS_ARG2_IMM(instr) 
        ? ARG2_VALUE(instr)
        : ramReadInt8(ram, code->constants[ARG2_VALUE(instr)]);
}


inline static float getArg2Float(Vm* vm, Bytecode* code, Instruction instr) {
    Ram* ram = vm->ram;
    Cpu32* cpu = vm->cpu;

    if(IS_ARG2_REG(instr)) {
        return (IS_ARG2_ADDR(instr)) 
            ? ramReadFloat(ram, cpu->regs[ARG2_VALUE(instr)].as.address)    
            : cpu->regs[ARG2_VALUE(instr)].as.fVal;
    }

    return ramReadFloat(ram, code->constants[ARG2_VALUE(instr)]);
}


void vmExecute(Vm* vm, Bytecode* code) {
    Cpu32* cpu = vm->cpu;
    Ram* ram = vm->ram;

    if(!code->length || !code->instrs) {
        return;
    }

#define INSTR_AT(index) (&code->instrs[(index)])

#define SET_ARG1_INT(instr,value)                                                 \
    SET_ARG1_INT_ARG(instr, ARG1_VALUE(instr), value)


#define SET_ARG1_INT_ARG(instr,argValue,value)                                    \
    do {                                                                          \
        if (IS_ARG1_ADDR(instr))                                                  \
            ramStoreInt32(ram, cpu->regs[(argValue)].as.address,(value));         \
        else cpu->regs[(argValue)].as.iVal = (value);                             \
    } while(0)


#define SET_ARG1_FLOAT(instr,value)                                               \
    SET_ARG1_FLOAT_ARG(instr,ARG1_VALUE(instr), value)

#define SET_ARG1_FLOAT_ARG(instr,argValue,value)                                  \
    do {                                                                          \
        if (IS_ARG1_ADDR(instr))                                                  \
            ramStoreFloat(ram, cpu->regs[(argValue)].as.address,(value));         \
        else cpu->regs[(argValue)].as.fVal = (value);                             \
    } while(0)


#define SET_ARG1_INT8(instr,value)                                                \
    SET_ARG1_INT8_ARG(instr,ARG1_VALUE(instr),value)

#define SET_ARG1_INT8_ARG(instr,argValue,value)                                   \
    do {                                                                          \
        if (IS_ARG1_ADDR(instr))                                                  \
            ramStoreInt8(ram, cpu->regs[(argValue)].as.address,(value));          \
        else cpu->regs[argValue].as.bVal = (value);                               \
    } while(0)

#define SET_ARG1_ADDR(instr,value)                                                \
    SET_ARG1_ADDR_ARG(instr,ARG1_VALUE(instr),value)

#define SET_ARG1_ADDR_ARG(instr,argValue,value)                                   \
    do {                                                                          \
        if (IS_ARG1_ADDR(instr))                                                  \
            ramStoreInt32(ram, cpu->regs[(argValue)].as.address,(value));         \
        else cpu->regs[(argValue)].as.address = (value);                          \
    } while(0)



#define GET_ARG1_INT(instr)                                        \
    ((IS_ARG1_ADDR(instr)) ?                                       \
        ramReadInt32(ram, cpu->regs[ARG1_VALUE(instr)].as.address) \
        : cpu->regs[ARG1_VALUE(instr)].as.iVal)

#define GET_ARG1_INT8(instr)                                       \
    ((IS_ARG1_ADDR(instr)) ?                                       \
        ramReadInt8(ram, cpu->regs[ARG1_VALUE(instr)].as.address)  \
        : cpu->regs[ARG1_VALUE(instr)].as.bVal)

#define GET_ARG1_FLOAT(instr)                                      \
    ((IS_ARG1_ADDR(instr)) ?                                       \
        ramReadFloat(ram, cpu->regs[ARG1_VALUE(instr)].as.address) \
        : cpu->regs[ARG1_VALUE(instr)].as.fVal)

#define GET_ARG2_INT(instr)                                        \
    getArg2Int32(vm, code, instr)

#define GET_ARG2_FLOAT(instr)                                      \
    getArg2Float(vm, code, instr)

#define GET_ARG2_INT8(instr)                                       \
    getArg2Int8(vm, code, instr)

#define GET_CONST_INT(instr)                                       \
    ((IS_ARG2_IMM(instr)) ?                                        \
        ARG2_VALUE(instr)                                          \
        : ramReadInt32(ram, code->constants[ARG2_VALUE(instr)]))

#define GET_CONST_INT8(instr)                                      \
    ((IS_ARG2_IMM(instr)) ?                                        \
        (int8_t)ARG2_VALUE(instr)                                  \
        : ramReadInt8(ram, code->constants[ARG2_VALUE(instr)]))

#define GET_CONST_FLOAT(instr)                                     \
    (ramReadFloat(ram, code->constants[ARG2_VALUE(instr)]))

#define GET_CONST_ADDR(instr)                                      \
    (code->constants[ARG2_VALUE(instr)])

#define OP_INT(instr,op)                                           \
    do {                                                           \
        int32_t aValue = GET_ARG1_INT(instr);                      \
        int32_t bValue = GET_ARG2_INT(instr);                      \
        int32_t result = aValue op bValue;                         \
        SET_ARG1_INT(instr, result);                               \
    } while(0)

#define OP_INT8(instr,op)                                          \
    do {                                                           \
        int8_t aValue = GET_ARG1_INT8(instr);                      \
        int8_t bValue = GET_ARG2_INT8(instr);                      \
        int8_t result = aValue op bValue;                          \
        SET_ARG1_INT8(instr, result);                              \
    } while(0)

#define OP_FLOAT(instr,op)                                         \
    do {                                                           \
        float aValue = GET_ARG1_FLOAT(instr);                      \
        float bValue = GET_ARG2_FLOAT(instr);                      \
        float result = aValue op bValue;                           \
        SET_ARG1_FLOAT(instr, result);                             \
    } while(0)    

#define CHECK_DIV_ZERO_INT(instr)                                  \
    do {                                                           \
        int32_t value = GET_ARG2_INT(instr);                       \
        if(value == 0) vmError("DivideByZeroError\n");             \
    } while(0)

#define CHECK_DIV_ZERO_INT8(instr)                                 \
    do {                                                           \
        int8_t value = GET_ARG2_INT8(instr);                       \
        if(value == 0) vmError("DivideByZeroError\n");             \
    } while(0)

#define CHECK_DIV_ZERO_FLOAT(instr)                                \
    do {                                                           \
        float value = GET_ARG2_FLOAT(instr);                       \
        if(value == 0) vmError("DivideByZeroError\n");             \
    } while(0)


    Instruction* pc = code->instrs;
    Instruction* end = INSTR_AT(code->length - 1);
            
    while(pc <= end) {
        cpu->pc.as.address = (Address)(pc - code->instrs); 

        Instruction instr = *pc++;        
        int32_t opcode = OPCODE(instr);
        
        
        //printf("Opcode: '%5s' Arg1: %5d Arg2: %5d PC: %5d  \n", 
        //    OpcodeStr[opcode], ARG1_VALUE(instr), ARG2_VALUE(instr), cpu->pc.as.address);

        switch(opcode) {
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
            case LDCI: {
                SET_ARG1_INT(instr, GET_CONST_INT(instr));
                break;
            }
            case LDCF: {
                SET_ARG1_FLOAT(instr, GET_CONST_FLOAT(instr));
                break;
            }
            case LDCB: {
                SET_ARG1_INT8(instr, GET_CONST_INT8(instr));
                break;
            }
            case LDCA: {
                SET_ARG1_ADDR(instr, GET_CONST_ADDR(instr));
                break;
            }
            case PUSHI: {
                int32_t value = GET_ARG2_INT(instr);

                cpu->sp.as.address -= ADDRESS_SIZE;
                ramStoreInt32(ram, cpu->sp.as.address, value);
                break;
            }
            case PUSHF: {
                float value = GET_ARG2_FLOAT(instr);

                cpu->sp.as.address -= ADDRESS_SIZE;
                ramStoreFloat(ram, cpu->sp.as.address, value);
                break;
            }
            case PUSHB: {
                int8_t value = GET_ARG2_INT8(instr);

                cpu->sp.as.address -= 1;
                ramStoreInt8(ram, cpu->sp.as.address, value);
                break;
            }
            case POPI: {
                int32_t value = ramReadInt32(ram, cpu->sp.as.address);
                cpu->sp.as.address += ADDRESS_SIZE;
                
                SET_ARG1_INT_ARG(instr, ARG2_VALUE(instr), value);
                break;
            }
            case POPF: {
                float value = ramReadFloat(ram, cpu->sp.as.address);
                cpu->sp.as.address += ADDRESS_SIZE;

                SET_ARG1_FLOAT_ARG(instr, ARG2_VALUE(instr), value);
                break;
            }
            case POPB: {
                int8_t value = ramReadInt8(ram, cpu->sp.as.address);
                cpu->sp.as.address += 1;

                SET_ARG1_INT8_ARG(instr, ARG2_VALUE(instr), value);
                break;
            }
            case DUPI: {
                int32_t value = ramReadInt32(ram, cpu->sp.as.address);
                cpu->sp.as.address -= ADDRESS_SIZE;
                ramStoreInt32(ram, cpu->sp.as.address, value);
                
                SET_ARG1_INT_ARG(instr, ARG2_VALUE(instr), value);
                break;
            }
            case DUPF: {
                float value = ramReadFloat(ram, cpu->sp.as.address);
                cpu->sp.as.address -= ADDRESS_SIZE;
                ramStoreFloat(ram, cpu->sp.as.address, value);
                
                SET_ARG1_FLOAT_ARG(instr, ARG2_VALUE(instr), value);
                break;
            }
            case DUPB: {
                int8_t value = ramReadInt8(ram, cpu->sp.as.address);
                cpu->sp.as.address -= 1;
                ramStoreInt8(ram, cpu->sp.as.address, value);
                
                SET_ARG1_INT8_ARG(instr, ARG2_VALUE(instr), value);
                break;
            }
            case IFI: {
                int32_t yValue = GET_ARG2_INT(instr);
                int32_t xValue = GET_ARG1_INT(instr);

                if(xValue > yValue) {
                    pc++;
                }

                break;
            }
            case IFF: {
                float yValue = GET_ARG2_FLOAT(instr);
                float xValue = GET_ARG1_FLOAT(instr);

                if(xValue > yValue) {
                    pc++;
                }

                break;
            }
            case IFB: {
                int8_t yValue = GET_ARG2_INT8(instr);
                int8_t xValue = GET_ARG1_INT8(instr);

                if(xValue > yValue) {
                    pc++;
                }

                break;
            }
            case IFEI: {
                int32_t yValue = GET_ARG2_INT(instr);
                int32_t xValue = GET_ARG1_INT(instr);

                if(xValue >= yValue) {
                    pc++;
                }

                break;
            }
            case IFEF: {
                float yValue = GET_ARG2_FLOAT(instr);
                float xValue = GET_ARG1_FLOAT(instr);

                if(xValue >= yValue) {
                    pc++;
                }

                break;
            }
            case IFEB: {
                int8_t yValue = GET_ARG2_INT8(instr);
                int8_t xValue = GET_ARG1_INT8(instr);

                if(xValue >= yValue) {
                    pc++;
                }

                break;
            }
            case PRINTI: {
                printf("%d", GET_ARG2_INT(instr));
                break;
            }
            case PRINTF: {
                printf("%f", GET_ARG2_FLOAT(instr));
                break;
            }
            case PRINTB: {
                printf("%d", GET_ARG2_INT8(instr));
                break;
            }
            case PRINTC: {
                printf("%c", (char)GET_ARG2_INT8(instr));
                break;
            }

            /* ===================================================
            * ALU operations 
            * ===================================================
            */
            case ADDI: {
                OP_INT(instr, +);
                break;
            }
            case ADDF: {
                OP_FLOAT(instr, +);
                break;
            }
            case ADDB: {
                OP_INT8(instr, +);
                break;
            }
            case SUBI: {
                OP_INT(instr, -);
                break;
            }
            case SUBF: {
                OP_FLOAT(instr, -);
                break;
            }
            case SUBB: {
                OP_INT8(instr, -);
                break;
            }
            case MULI: {
                OP_INT(instr, *);
                break;
            }
            case MULF: {
                OP_FLOAT(instr, *);
                break;
            }
            case MULB: {
                OP_INT8(instr, *);
                break;
            }
            case DIVI: {
                CHECK_DIV_ZERO_INT(instr);
                OP_INT(instr, /);
                break;
            }
            case DIVF: {
                CHECK_DIV_ZERO_FLOAT(instr);
                OP_FLOAT(instr, /);
                break;
            }
            case DIVB: {
                CHECK_DIV_ZERO_INT8(instr);
                OP_INT8(instr, /);
                break;
            }
            case MODI: {
                CHECK_DIV_ZERO_INT(instr);
                OP_INT(instr, %);
                break;
            }
            case MODF: {
                CHECK_DIV_ZERO_FLOAT(instr);
                
                float aValue = GET_ARG1_FLOAT(instr);
                float bValue = GET_ARG2_FLOAT(instr);
                float result = (int)aValue % (int)bValue;
                SET_ARG1_FLOAT(instr, result);    
                break;
            }
            case MODB: {
                CHECK_DIV_ZERO_INT8(instr);
                OP_INT8(instr, %);
                break;
            }
            case ORI: {
                OP_INT(instr, |);
                break;
            }
            case ORB: {
                OP_INT8(instr, |);
                break;
            }
            case ANDI: {
                OP_INT(instr, &);
                break;
            }
            case ANDB: {
                OP_INT8(instr, &);
                break;
            }
            case NOTI: {
                int32_t value = ~GET_ARG2_INT(instr);
                SET_ARG1_INT(instr, value);
                break;
            }
            case NOTB: {
                int8_t value = ~GET_ARG2_INT8(instr);
                SET_ARG1_INT8(instr, value);
                break;
            }
            case XORI: {
                OP_INT(instr, ^);
                break;
            }
            case XORB: {
                OP_INT8(instr, ^);
                break;
            }
            case SZRLI: {
                OP_INT(instr, >>);
                break;
            }
            case SZRLB: {
                OP_INT8(instr, >>);
                break;
            }
            case SRLI: {
                OP_INT(instr, >>);
                break;
            }
            case SRLB: {
                OP_INT8(instr, >>);
                break;
            }
            case SLLI: {
                OP_INT(instr, <<);
                break;
            }
            case SLLB: {
                OP_INT8(instr, <<);
                break;
            }
            default: {
                vmError("Unknown opcode: %d\n", opcode);
            }
        }
    }

#undef INSTR_AT 
#undef SET_ARG1_INT   
#undef SET_ARG1_FLOAT
#undef SET_ARG1_INT8
#undef SET_ARG1_ADDR
#undef SET_ARG1_INT_ARG   
#undef SET_ARG1_FLOAT_ARG
#undef SET_ARG1_INT8_ARG
#undef SET_ARG1_ADDR_ARG
#undef GET_ARG1_INT
#undef GET_ARG1_FLOAT
#undef GET_ARG2_INT
#undef GET_ARG2_FLOAT
#undef GET_ARG2_INT8
#undef GET_CONST_INT
#undef GET_CONST_INT8
#undef GET_CONST_FLOAT
#undef GET_CONST_ADDR
#undef OP_INT
#undef OP_INT8
#undef OP_FLOAT
#undef CHECK_DIV_ZERO_INT
#undef CHECK_DIV_ZERO_INT8
#undef CHECK_DIV_ZERO_FLOAT
}
