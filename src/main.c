
#define __USE_MINGW_ANSI_STDIO 1
#define _CRT_SECURE_NO_WARNINGS

// standard includes
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// program includes
#include "common.c"
#include "buf.c"
#include "bytecode.c"
#include "assembler.c"
#include "vm.c"

int main(int argc, char** argv) {
    Vm* vm = vmInit(1024, 1024 * 1024);

    //printf("Hello world\n");
/*
    AssemblerInstruction* instrs = parse("Hello Bye\nWorld Gone\n");
    while(instrs) {
        for(int i = 0; i < instrs->numberOfArgs; i++) {
            if(i > 0) {
                printf(",");
            }
            printf("%s", instrs->args[i]);
        }

        printf(";");

        instrs = instrs->next;
    }
*/
    const char* assembly =
        ";; this is a comment \n"
        ".text \"Test\" \n"
        "ldca $a .text  \n" 
        "pushi $a  \n"
        "call :print_string  \n"
        "jmp :exit \n"
        "printi #11  \n"
        ":print_string         \n"
        "        popi $a      \n"
        "    :print_loop  \n"
        "        ifb &$a #0        \n"
        "        jmp :print_end_loop  \n"
        "        printc &$a        \n"
        "        addi $a #1        \n"
        "        jmp :print_loop \n"
        "    :print_end_loop     \n"
        "        ret \n"
        "      \n"
        ":exit \n"
        ;

    Bytecode* code = compile(vm, assembly);
    disassemble(code);
    
    vmExecute(vm, code);
    vmFree(vm);

    return 0;
}