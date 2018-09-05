#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "assembler.h"
#include "common.h"
#include "bytecode.h"


int cpuGetRegisterIndex(Cpu32* cpu, const char* name);

typedef struct AssemblerInstruction {
    char** args;
    size_t numberOfArgs;

    size_t lineNumber;

    struct AssemblerInstruction* next;  
} AssemblerInstruction;


typedef struct Program {
    AssemblerInstruction* instrs;
    size_t numberOfInstructions;
} Program;

typedef enum ConstantKind {
    STRING,
    INT32,
    INT8,
    FLOAT
} ConstantKind;

typedef struct Constant {
    char*        name;
    size_t       index;
    ConstantKind kind;
    union {
        int32_t int32Val;
        int8_t  int8Val;
        float   floatVal;
        char*   stringVal;
    } as;
} Constant;


static void parseError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
}

static AssemblerInstruction* makeAssemblerInstruction(size_t lineNumber) {
    AssemblerInstruction* instr = litaMalloc(sizeof(AssemblerInstruction));
    instr->lineNumber = lineNumber;
    instr->numberOfArgs = 0;
    instr->args = NULL;
    instr->next = NULL;
    return instr;
}

static Instruction parseArg1(Vm* vm, AssemblerInstruction* instr, char* arg) {
    return 0;
}

static Instruction parseArg2(Vm* vm, AssemblerInstruction* instr, char* arg) {
    size_t argLen = strlen(arg);
    
    Instruction instruction = 0;
    int isRegister = 0;
    int isAddress = (arg[0] == '&') ? 1 : 0;


    if(isAddress) {
        isRegister = 1;
        instruction |= ARG2_ADDR_MASK;

        if(argLen < 3) {
            parseError("Invalid argument structure: '%s' at line: %d", arg, instr->lineNumber);
        }

        arg++; // eat the &
    }

    int registerIndex = cpuGetRegisterIndex(vm->cpu, arg);
    if(registerIndex > -1) {
        isRegister = 1;
    }

    return instruction;
}

static Instruction parseJmp(Vm* vm, AssemblerInstruction* instr, char* arg) {
    return 0;
}

static Instruction* parseInstructions(Vm* vm, Program* program) {

    AssemblerInstruction* instrs = program->instrs;
    Instruction* result = (Instruction*)litaMalloc(sizeof(Instruction) * program->numberOfInstructions);

    size_t i = 0;
    while(instrs) {
        if(instrs->numberOfArgs > 0 && instrs->args) {
            char* opcodeStr = instrs->args[0];
            Opcode opcode = opcodeFromString(opcodeStr);
            if(opcode < 0) {
                parseError("Invalid opcode: '%s' at line: %d", opcodeStr, instrs->lineNumber);
            }
            
            size_t expectedNumArgs = opcodeNumArgs(opcode);            
            if((instrs->numberOfArgs - 1) != expectedNumArgs) {
                parseError("Invalid number of arguments '%d', expected '%d' for opcode: '%s' at line: %d", 
                    (instrs->numberOfArgs - 1), expectedNumArgs, opcodeStr, instrs->lineNumber);                        
            }

            Instruction instruction = opcode << (ARG1_SIZE + ARG2_SIZE);

            Instruction arg1 = 0;
            Instruction arg2 = 0;

            switch(opcode) {
                case JMP:
                case CALL: {
                    arg2 = parseJmp(vm, instrs, instrs->args[1]);
                    break;
                }
                default: {
                    
                    switch(expectedNumArgs) {                
                        case 1: {
                            // parse with arg2 format options
                            arg2 = parseArg2(vm, instrs, instrs->args[1]);
                            break;
                        }
                        case 2: {
                            arg1 = parseArg1(vm, instrs, instrs->args[1]);
                            arg2 = parseArg2(vm, instrs, instrs->args[2]);
                            break;
                        }
                        default: {
                            parseError("Invalid number of arguments '%d' for opcode: '%s' at line: %d", 
                                instrs->numberOfArgs, opcodeStr, instrs->lineNumber);
                        }
                    }
                }

            }
            
            result[i++] = instruction | arg1 | arg2;
        }
        
        instrs = instrs->next;                
    }

    return result;
}

#define MAX_CONSTANTS 1024
#define MAX_INT32_VALUE 256

static void parseConstant(AssemblerInstruction* instrs, Constant* constant, char* arg, size_t argLen) {
    if(!arg || argLen < 1) {
        parseError("Constant expression value can not be empty at line: %d", instrs->lineNumber);
    }
    
    if(arg[0] == '\"') {
        if(arg[argLen] != '\"') {
            parseError("Constant string expression missing closing '\"' at line: %d", instrs->lineNumber);
        }

        constant->kind = STRING;
        constant->as.stringVal = arg;        
    }
    else {
        // TODO: Support 0b, 0x and e number formats, specify number type?
        int hasDecimal = 0;    
        int hasNegative = 0;

        for(size_t i = 0; i < argLen; i++) {
            char c = arg[i];
            if(c == '.') {
                if(hasDecimal) {
                    parseError("Invalid constant number expression contains multiple decimals at line: %d", instrs->lineNumber);
                }

                hasDecimal++;
            }
            else if(c == '-') {
                if(hasNegative) {
                    parseError("Invalid constant number expression contains multiple negatives at line: %d", instrs->lineNumber);
                }

                hasNegative++;
            }
            else {
                if(!isdigit(c)) {
                    parseError("Invalid constant number expression at line: %d", instrs->lineNumber);
                }
            }
        }
        
        if(hasDecimal) {
            constant->kind = FLOAT;
            constant->as.floatVal = atof(arg);
        }
        else {
            int num = atoi(arg);
            if(num > INT32_MAX || num < INT32_MIN) {
                parseError("Invalid constant number expression out of range at line: %d", instrs->lineNumber);
            }

            if(num <= INT8_MAX && num >= INT8_MIN) {
                constant->kind = INT8;
                constant->as.int8Val = num;                
            }
            else {
                constant->kind = INT32;
                constant->as.int32Val = num;                
            }
        }
    }
}

static Address* parseConstants(Vm* vm, Program* program) {
    Address* result = NULL;
    Constant constants[MAX_CONSTANTS];
    size_t numConstants = 0;

    AssemblerInstruction* instrs = program->instrs;

    while(instrs) {
        if(instrs->numberOfArgs > 0 && instrs->args) {
            char* opcode = instrs->args[0];
            size_t len = strlen(opcode);
            if(opcode && len > 0) {
                if(opcode[0] == '.') {
                    if(instrs->numberOfArgs < 2) {
                        parseError("Illegal constant expression: %s at line: %d", opcode, instrs->lineNumber);
                    }

                    size_t index = 0;

                    char* arg = instrs->args[1];
                    size_t argLen = strlen(arg);

                    if((numConstants + 1) > MAX_CONSTANTS) {
                        parseError("Exceeded max number of constants: '%d' at line: $d", MAX_CONSTANTS, instrs->lineNumber);
                    }

                    Constant c = constants[numConstants++];
                    c.index = index++;
                    c.name = opcode;
                    parseConstant(instrs, &c, arg, argLen);
                }
            }
        }
        
        instrs = instrs->next;        
    }

    Address ramAddress = 0;
    if(numConstants > 0) {
        result = (Address*)litaMalloc(sizeof(Address) * numConstants);
        Ram* ram = vm->ram;
        for(size_t i = 0; i < numConstants; i++) {
            result[i] = ramAddress;
            Constant c = constants[i];
            switch(c.kind) {
                case INT32: {
                    ramStoreInt32(ram, ramAddress, c.as.int32Val);
                    ramAddress += 4;
                    break;
                }
                case FLOAT: {                    
                    ramStoreFloat(ram, ramAddress, c.as.floatVal);
                    ramAddress += 4;
                    break;
                }
                case INT8: {
                    ramStoreInt8(ram, ramAddress, c.as.int8Val);
                    ramAddress += 1;
                    break;
                }
                case STRING: {
                    size_t len = strlen(c.as.stringVal);
                    ramStoreString(ram, ramAddress, c.as.stringVal, len);
                    ramAddress += len + 1;
                    break;
                }
            }
        }

    }
    vm->cpu->h.as.address = ramAddress;

    return result;
}

static AssemblerInstruction* parseLine(size_t lineNumber, const char* line, const char* end) {    
    int inComment = 0;
    int inString  = 0;
    char* buf = NULL;
    
    // trim leading spaces
    for(; *line == ' ' && line < end; line++);
    if(line >= end) {
        return NULL;
    }

    AssemblerInstruction* instr = makeAssemblerInstruction(lineNumber);

    for(;line < end; line++) {
        char c = *line;
                
        if(c == ';') {
            if(!inString) {
                inComment = 1;
            }
        }
        else if(c == '"') {
            if(inString) {
                inString = 1;
                continue;
            }
            else if(!inComment) {
                inString = 1;
                continue;
            }
        }
        else if(c == ' ') {
            if(!inString && !inComment) {                                            
                buf_push(buf, 0);                
                buf_push(instr->args, buf);
                instr->numberOfArgs++;                

                buf = NULL;
                continue;
            }
        }

        if(!inComment) {            
            buf_push(buf, c);            
        }
    }

    buf_push(buf, 0);                    
    buf_push(instr->args, buf);
    instr->numberOfArgs++;      

    buf_push(instr->args, 0);
    
    return instr;
}



void parse(Program* program, const char* assembly) {
    size_t lineNumber = 1;
    AssemblerInstruction* current = NULL;

    const char* start = assembly;
    while(*assembly) {
        char c = *assembly;
        if(c == '\n') {
            AssemblerInstruction* next = parseLine(lineNumber, start, assembly);
            program->numberOfInstructions++;

            if(next) {
                if(program->instrs == NULL)  {
                    program->instrs = next;                    
                }
                else {
                    current->next = next;
                }

                current = next;
            }

            lineNumber++;
            start = assembly + 1;
        }

        assembly++;
    }
}

Bytecode* compile(Vm* vm, const char* assembly) {
    Program program = {
        .instrs = NULL,
        .numberOfInstructions = 0
    };

    parse(&program, assembly);
        
    Address* constants = parseConstants(vm, &program);
    Instruction* instructions = parseInstructions(vm, &program);

    return NULL;
}


void      bytecodeFree(Bytecode* code) {
    if(code) {
        litaFree(code->constants);
        litaFree(code->instrs);
        litaFree(code);
    }
}