
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

const char* USAGE =
"<usage> litavm [options] file\n"
        "Options: \n"
        "  -d\t\tShows disassembly output\n"
        "\n\nExample:\n"
        "\tlitavm -d /scripts/hello.asm"
;        

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("%s", USAGE);
        return 0;
    }

    Vm* vm = vmInit(1024, 1024 * 1024);

    // const char* assembly =
    //     ";; this is a comment \n"
    //     ".text \"Test\" \n"
    //     "ldca $a .text  \n" 
    //     "pushi $a  \n"
    //     "call :print_string  \n"
    //     "jmp :exit \n"
    //     "printi #11  \n"
    //     ":print_string         \n"
    //     "        popi $a      \n"
    //     "    :print_loop  \n"
    //     "        ifb &$a #0        \n"
    //     "        jmp :print_end_loop  \n"
    //     "        printc &$a        \n"
    //     "        addi $a #1        \n"
    //     "        jmp :print_loop \n"
    //     "    :print_end_loop     \n"
    //     "        ret \n"
    //     "      \n"
    //     ":exit \n"
    //     ;
    int displayDisassembly = 0;
    const char* filename = NULL;
    for(size_t i = 1; i < argc; i++) {
        if(!strcmp("-d", argv[i])) {
            displayDisassembly = 1;
        }
        else {
            filename = argv[i];
        }
    }

    if(!filename) {
        printf("%s", USAGE);
        return 0;
    }
    
    const char* assembly = readFile(filename);
    
    Bytecode* code = compile(vm, assembly);
    if(displayDisassembly) {
        disassemble(code);
    }
    
    vmExecute(vm, code);
    vmFree(vm);

    return 0;
}