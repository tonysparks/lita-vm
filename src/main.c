
#define __USE_MINGW_ANSI_STDIO 1
#define _CRT_SECURE_NO_WARNINGS

// standard includes
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// program includes
/*#include "common.h"
#include "buf.h"
#include "parser.h"
#include "bytecode.h"
#include "assembler.h"
*/

#include "common.c"
#include "buf.c"
#include "parser.c"
#include "assembler.c"

#include "vm.c"

int main(int argc, char** argv) {
    //printf("Hello world\n");

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

    const char* text = "Packers";

    Ram* ram = ramInit(1024);
    ramStoreString(ram, 0, text, strlen(text));
    char read[512];
    ramReadBytes(ram, 0, &read, strlen(text));

    printf("Text: %s \n", read);

    return 0;
}