// standard includes
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "buf.h"
#include "parser.h"


static AssemblerInstruction* makeAssemblerInstruction(size_t lineNumber) {
    AssemblerInstruction* instr = litaMalloc(sizeof(AssemblerInstruction));
    instr->lineNumber = lineNumber;
    instr->numberOfArgs = 0;
    instr->args = NULL;
    instr->next = NULL;
    return instr;
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

static void parseError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
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