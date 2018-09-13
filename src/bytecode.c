#include <stdint.h>
#include "bytecode.h"
#include "common.h"


Opcode opcodeFromString(const char* opcodeStr) {
    for(size_t i = 0; i < MAX_OPCODES; i++) {
        if(!strCmpIgnoreCase(OpcodeStr[i], opcodeStr)) {
            return i;
        }
    }

    return -1;
}

size_t opcodeNumArgs(Opcode opcode) {
    switch(opcode) {
        case RET:
        case NOOP: 
            return 0;
        case PUSHI:
        case PUSHF:
        case PUSHB:
        case POPI:
        case POPF:
        case POPB:
        case DUPI:
        case DUPF:
        case DUPB:
        case JMP:
        case PRINTI:
        case PRINTF:
        case PRINTB:
        case PRINTC:
        case CALL:
            return 1;
        default: 
            return 2;
    }
}

void bytecodeFree(Bytecode* code) {
    if(code) {
        litaFree(code->constants);
        litaFree(code->instrs);
        litaFree(code);
    }
}