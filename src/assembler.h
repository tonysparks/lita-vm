#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "bytecode.h"
#include "vm.h"

Bytecode* compile(Vm* vm, const char* assembly);
void      disassemble(Bytecode* code);

#endif
