#include <stdint.h>

#include "assembler.h"
#include "common.h"
#include "bytecode.h"

typedef struct AssemblerInstruction {
    char** args;
    size_t numberOfArgs;

    size_t lineNumber;

    struct AssemblerInstruction* next;  
} AssemblerInstruction;

typedef struct Constant {
    char*    name;
    size_t   index;
    int      isNumber;
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

static Instruction parseAssemblerInstruction(AssemblerInstruction* instr) {
    return 0;
}

static int32_t* buildConstants(AssemblerInstruction* instrs) {
    int32_t* result = NULL;

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

                    // TODO: Parse out Number/String
                }
            }
        }
        
        instrs = instrs->next;        
    }

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



AssemblerInstruction* parse(const char* assembly) {
    size_t lineNumber = 1;
    AssemblerInstruction* result  = NULL;
    AssemblerInstruction* current = NULL;

    const char* start = assembly;

    while(*assembly) {
        char c = *assembly;
        if(c == '\n') {
            AssemblerInstruction* next = parseLine(lineNumber, start, assembly);
            if(next) {
                if(result == NULL)  {
                    result = next;                    
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

    return result;
}

Bytecode* compile(const char* assembly) {
    AssemblerInstruction* instrs = parse(assembly);
    

    return NULL;
}