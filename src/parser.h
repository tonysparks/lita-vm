#ifndef PARSER_H
#define PARSER_H

typedef struct AssemblerInstruction {
    char** args;
    size_t numberOfArgs;

    size_t lineNumber;

    struct AssemblerInstruction* next;  
} AssemblerInstruction;

AssemblerInstruction* parse(const char* assembly);

#endif