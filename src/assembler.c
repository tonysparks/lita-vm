#include <stdint.h>

#include "assembler.h"
#include "bytecode.h"
#include "parser.h"



Bytecode* compile(const char* assembly) {
    AssemblerInstruction* instrs = parse(assembly);
    

    return NULL;
}