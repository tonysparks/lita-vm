//#define __USE_MINGW_ANSI_STDIO 1
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
        "  -d,--disassembly         Shows disassembly output\n"
        "  -s,--stack-size          Set the max stack size.  Defaults to 1024 bytes\n"
        "  -r,--ram                 Set the amount of RAM in bytes.  Defaults to 1 MiB\n"
        "\n\nExample:\n"
        "\tlitavm -d -s 4096 /scripts/hello.asm"
;        

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("%s", USAGE);
        return 0;
    }

    VmConfig config;
    config.ramSize = 1024 * 1024;
    config.stackSize = 1024;

    int displayDisassembly = 0;
    const char* filename = NULL;

    for(int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if(!strcmp("-d", arg) || !strcmp("--disassembly", arg)) {
            displayDisassembly = 1;
        }
        else if(!strcmp("-s", arg) || !strcmp("--stack-size", arg)) {
            if((i + 1) >= argc) {
                vmError("Invalid number of parameters, must have a number after stack-size");
            }
            const char* param = argv[i+1];
            config.stackSize = (size_t) strtol(param, NULL, 10);
            i++;
        }
        else if(!strcmp("-r", arg) || !strcmp("--ram", arg)) {
            if((i + 1) >= argc) {
                vmError("Invalid number of parameters, must have a number after ram");
            }
            const char* param = argv[i+1];
            config.ramSize = (size_t) strtol(param, NULL, 10);
            i++;
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
    
    Vm* vm = vmInit(&config);
    Bytecode* code = compile(vm, assembly);

    if(displayDisassembly) {
        disassemble(code);
    }
    
    vmExecute(vm, code);
    vmFree(vm);

    return 0;
}
