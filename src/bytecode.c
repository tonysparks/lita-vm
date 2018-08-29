#include <stdint.h>
#include "bytecode.h"

Opcode fromString(const char* opcodeStr) {
    for(size_t i = 0; i < MAX_OPCODES; i++) {
        if(strcmp(OpcodeStr[i], opcodeStr)) {
            return i;
        }
    }

    return -1;
}