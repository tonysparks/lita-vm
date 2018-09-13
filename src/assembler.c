#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "assembler.h"
#include "common.h"
#include "bytecode.h"

#define MAX_INT32_VALUE 256

int cpuGetRegisterIndex(const char* name);



typedef enum InstructionKind {
    UNKNOWN,
    CONSTANT_DEF,
    LABEL_DEF,
    BYTECODE_DEF,
} InstructionKind;

typedef struct AssemblerInstruction {
    char** args;
    size_t numberOfArgs;

    size_t  lineNumber;
    Address address;     /* the instruction address */

    InstructionKind kind;    
    struct AssemblerInstruction* next;  
} AssemblerInstruction;

typedef enum ConstantKind {
    STRING,
    INT32,
    INT8,
    FLOAT
} ConstantKind;

typedef struct Constant {
    char*        name;
    size_t       index;
    ConstantKind kind;
    union {
        int32_t int32Val;
        int8_t  int8Val;
        float   floatVal;
        char*   stringVal;
    } as;

    struct Constant* next;
} Constant;

typedef struct Label {
    char*   name;
    Address address;
    struct Label* next;
} Label;

typedef struct Program {
    AssemblerInstruction* instrs;
    size_t numberOfInstructions;

    Constant* constants;
    size_t numberOfConstants;

    Label*  labels;
    size_t  numberOfLabels;
} Program;

static void parseError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    exit(32);
}

static AssemblerInstruction* makeAssemblerInstruction(size_t lineNumber) {
    AssemblerInstruction* instr = litaMalloc(sizeof(AssemblerInstruction));
    instr->lineNumber = lineNumber;
    instr->numberOfArgs = 0;
    instr->args = NULL;
    instr->next = NULL;
    instr->address = 0;
    instr->kind = UNKNOWN;
    return instr;
}

static void freeAssemberInstruction(AssemblerInstruction* instr) {
    AssemblerInstruction* i = instr;
    while(i) {
        for(size_t k = 0; k < i->numberOfArgs; k++) {
            buf_free(i->args[k]);
        }
        buf_free(i->args);

        AssemblerInstruction* next = i->next;
        litaFree(i);

        i = next;
    }
}

static void freeConstants(Constant* constants) {
    Constant* i = constants;
    while(i) {
        Constant* next = i->next;
        litaFree(i);

        i = next;
    }
}

static void freeLabels(Label* labels) {
    Label* i = labels;
    while(i) {
        Label* next = i->next;
        litaFree(i);

        i = next;
    }
}

static int findConstantIndex(Program* program, const char* constantName) {
    Constant* i = program->constants;
    while(i) {
        if(!strcmp(i->name, constantName)) {
            return i->index;
        }

        i = i->next;
    }

    return -1;
}


static Label* findLabel(Program* program, const char* labelName) {
    Label* c = program->labels;
    while(c) {
        if(!strcmp(c->name, labelName)) {
            return c;
        }

        c = c->next;
    }

    return NULL;
}

static int32_t parseImmediateNumber(AssemblerInstruction* instr, char* arg, size_t argLen) {    
    size_t base = 10;
    size_t offset = 1;

    if(strStartsWith("#0x", arg)) {
        base = 16;
        offset = 3;
    }
    else if(strStartsWith("#0b", arg)) {
        base = 2;
        offset = 3;
    }

    if(argLen < (offset - 1)) {
        parseError("Invalid immediate value argument structure: '%s' at line: %d", arg, instr->lineNumber);
    }

    arg = &arg[offset];
    int32_t value = strtol(arg, NULL, base);

    if(value > MAX_IMMEDIATE_VALUE) {
        parseError("Invalid immediate value '%d', above max value of '%d' at line: %d", value, MAX_IMMEDIATE_VALUE, instr->lineNumber);
    }

    return value;
}


static Instruction parseArg1(AssemblerInstruction* instr, char* arg) {
    size_t argLen = strlen(arg);
    
    Instruction instruction = 0;    
    int isAddress = (arg[0] == '&') ? 1 : 0;


    if(isAddress) {        
        instruction |= ARG1_ADDR_MASK;

        if(argLen < 3) {
            parseError("Invalid argument structure: '%s' at line: %d", arg, instr->lineNumber);
        }

        arg++; // eat the &
    }

    int registerIndex = cpuGetRegisterIndex(arg);
    if(registerIndex < 0) {
        parseError("Invalid register name: '%s' at line: %d", arg, instr->lineNumber);            
    }
        
    instruction |= registerIndex;

    return instruction << ARG2_SIZE;
}

static Instruction parseArg2(Program* program, AssemblerInstruction* instr, char* arg) {    
    size_t argLen = strlen(arg);
    
    Instruction instruction = 0;
    int isRegister = 0;
    int isAddress = (arg[0] == '&') ? 1 : 0;

    if(isAddress) {
        isRegister = 1;
        instruction |= ARG2_ADDR_MASK;

        if(argLen < 3) {
            parseError("Invalid argument structure: '%s' at line: %d", arg, instr->lineNumber);
        }

        arg++; // eat the &
        argLen--;
    }

    
    int registerIndex = cpuGetRegisterIndex(arg);
    if(registerIndex > -1) {
        isRegister = 1;
        instruction |= registerIndex;
    }
    else {
        if(isAddress) {
            parseError("Invalid register argument structure: '%s' at line: %d", arg, instr->lineNumber);
        }

        // if this is a label, convert it into an actual address pointer
        if(arg[0] == ':') {
            Label* label = findLabel(program, arg);
            if(!label) {
                parseError("Invalid label: '%s' at line: %d", arg, instr->lineNumber);
            }

            instruction |= ARG2_IMM_MASK;
            instruction |= label->address;
        }
        // if this is an immediate mode value, parse it out
        else if(arg[0] == '#') {            
            int32_t value = parseImmediateNumber(instr, arg, argLen);

            instruction |= ARG2_IMM_MASK;
            instruction |= value;
        }
        // if this is a constant, look up the constant index
        else if(arg[0] == '.') {            
            char* constantName = arg;
            int index = findConstantIndex(program, constantName);
            if(index < 0) {
                parseError("No constant defined for '%s' at line: %d", constantName, instr->lineNumber);
            }

            instruction |= index;
        }
    }

    if(isRegister) {
        instruction |= ARG2_REG_MASK;
    }

    return instruction;
}

static Instruction parseJmp(Program* program, AssemblerInstruction* instr, char* arg) {
    if(arg[0] == ':') {
        Label* label = findLabel(program, arg);
        if(!label) {
            parseError("Invalid label: '%s' at line: %d", arg, instr->lineNumber);
        }

        return label->address;
    }
    else if(arg[0] == '#') {
        arg++;
        size_t argLen = strlen(arg);
        int32_t value = parseImmediateNumber(instr, arg, argLen);

        return value;
    }
    
    parseError("Invalid jump instruction argument, must be an immedate number or label: '%s' at line: %d", arg, instr->lineNumber);
    
    return 0;
}

static Instruction* parseInstructions(Program* program) {
    AssemblerInstruction* instrs = program->instrs;
    Instruction* result = (Instruction*)litaMalloc(sizeof(Instruction) * (program->numberOfInstructions + 1));
    
    size_t i = 0;
    while(instrs) {
        if(instrs->kind == BYTECODE_DEF) {
            if(instrs->numberOfArgs > 0 && instrs->args) {
                char* opcodeStr = instrs->args[0];
                Opcode opcode = opcodeFromString(opcodeStr);
                
                if(opcode < 0) {
                    parseError("Invalid opcode: '%s' at line: %d", opcodeStr, instrs->lineNumber);
                }
                
                size_t expectedNumArgs = opcodeNumArgs(opcode);            
                if((instrs->numberOfArgs - 1) != expectedNumArgs) {
                    parseError("Invalid number of arguments '%d', expected '%d' for opcode: '%s' at line: %d", 
                        (instrs->numberOfArgs - 1), expectedNumArgs, opcodeStr, instrs->lineNumber);                        
                }

                Instruction instruction = opcode << (ARG1_SIZE + ARG2_SIZE);

                Instruction arg1 = 0;
                Instruction arg2 = 0;

                switch(opcode) {
                    case JMP:
                    case CALL: {
                        arg2 = parseJmp(program, instrs, instrs->args[1]);
                        break;
                    }
                    default: {
                        
                        switch(expectedNumArgs) {           
                            case 0: {
                                break;     
                            }
                            case 1: {
                                // parse with arg2 format options
                                arg2 = parseArg2(program, instrs, instrs->args[1]);
                                break;
                            }
                            case 2: {
                                arg1 = parseArg1(instrs, instrs->args[1]);
                                arg2 = parseArg2(program, instrs, instrs->args[2]);
                                break;
                            }
                            default: {
                                parseError("Invalid number of arguments '%d' for opcode: '%s' at line: %d", 
                                    instrs->numberOfArgs, opcodeStr, instrs->lineNumber);
                            }
                        }
                    }

                }
                
                result[i++] = instruction | arg1 | arg2;
            }
        }
        instrs = instrs->next;                
    }

    result[i] = NOOP; // end marker

    return result;
}

static void parseConstant(AssemblerInstruction* instrs, Constant* constant, char* arg, size_t argLen) {
    if(!arg || argLen < 1) {
        parseError("Constant expression value can not be empty at line: %d", instrs->lineNumber);
    }
    
    if(arg[0] == '\"') {
        if(argLen > 1 && arg[argLen - 1] != '\"') {
            parseError("Constant string expression missing closing '\"' at line: %d", instrs->lineNumber);
        }

        char* str = (char*)litaMalloc(sizeof(char) * (argLen - 1));
        memcpy(str, arg + 1, argLen - 2);
        str[argLen - 1] = 0;
        
        constant->kind = STRING;
        constant->as.stringVal = str;
    }
    else {
        int hasDecimal = 0;
        int base = 10;

        if(strStartsWith("0x", arg)) {            
            base = 16;

            for(size_t i = 2; i < argLen; i++) {
                char c = arg[i];
                if(!isxdigit(c)) {
                    parseError("Invalid constant hexidecimal number expression '%s' at line: %d", arg, instrs->lineNumber);
                }
            }

            arg += 2;
        }
        else if(strStartsWith("0b", arg)) {
            base = 2;

            for(size_t i = 2; i < argLen; i++) {
                char c = arg[i];
                if(c != '0' && c != '1') {
                    parseError("Invalid constant binary number expression '%s' at line: %d", arg, instrs->lineNumber);
                }
            }

            arg += 2;
        }
        else {
            int hasNegative = 0;

            for(size_t i = 0; i < argLen; i++) {
                char c = arg[i];
                if(c == '.') {
                    if(hasDecimal) {
                        parseError("Invalid constant number expression '%s' contains multiple decimals at line: %d", arg, instrs->lineNumber);
                    }

                    hasDecimal++;
                }
                else if(c == '-') {
                    if(hasNegative) {
                        parseError("Invalid constant number expression '%s' contains multiple negatives at line: %d", arg, instrs->lineNumber);
                    }

                    hasNegative++;
                }
                else {
                    if(!isdigit(c)) {
                        parseError("Invalid constant number expression '%s' at line: %d", arg, instrs->lineNumber);
                    }
                }
            }
        }
            
        if(hasDecimal) {
            constant->kind = FLOAT;
            constant->as.floatVal = atof(arg);
        }
        else {            
            int32_t num = strtol(arg, NULL, base);
            if(num > INT32_MAX || num < INT32_MIN) {
                parseError("Invalid constant number expression out of range at line: %d", instrs->lineNumber);
            }

            if(num <= INT8_MAX && num >= INT8_MIN) {
                constant->kind = INT8;
                constant->as.int8Val = num;
            }
            else {
                constant->kind = INT32;
                constant->as.int32Val = num;  
            }
        }
    }
}


static Address* parseConstants(Vm* vm, Program* program) {
    Address* result = NULL;
    AssemblerInstruction* instrs = program->instrs;
    Constant* current = NULL;

    size_t index = 0;
    while(instrs) {
        if(instrs->kind == CONSTANT_DEF) {
            char* name = instrs->args[0];
            
            if(instrs->numberOfArgs < 2) {
                parseError("Illegal constant expression: %s at line: %d", name, instrs->lineNumber);
            }

            char* arg = instrs->args[1];
            size_t argLen = strlen(arg);
            
            Constant* c = (Constant*)litaMalloc(sizeof(Constant));            
            c->index = index++;
            c->name = name;            
            parseConstant(instrs, c, arg, argLen);

            if(!program->constants) {            
                program->constants = c;
            }
            else {
                current->next = c;
            }

            current = c;
            program->numberOfConstants++;
        }
        
        instrs = instrs->next;        
    }
    Address ramAddress = 0;
    if(program->numberOfConstants > 0) {
        result = (Address*)litaMalloc(sizeof(Address) * program->numberOfConstants);        
        Ram* ram = vm->ram;
        Constant* c = program->constants;
        size_t i = 0;
        while(c) {                        
            result[i++] = ramAddress;
            switch(c->kind) {
                case INT32: {
                    ramStoreInt32(ram, ramAddress, c->as.int32Val);
                    ramAddress += 4;
                    break;
                }
                case FLOAT: {                    
                    ramStoreFloat(ram, ramAddress, c->as.floatVal);
                    ramAddress += 4;
                    break;
                }
                case INT8: {                    
                    ramStoreInt8(ram, ramAddress, c->as.int8Val);
                    ramAddress += 1;
                    break;
                }
                case STRING: {
                    size_t len = strlen(c->as.stringVal);
                    ramStoreString(ram, ramAddress, c->as.stringVal, len);
                    ramAddress += len + 1;
                    break;
                }
            }

            c = c->next;
        }

    }
    vm->cpu->h.as.address = ramAddress;

    return result;
}


static void parseLabels(Program* program) {    
    AssemblerInstruction* instrs = program->instrs;
    Label* current = NULL;
    while(instrs) {
        if(instrs->kind == LABEL_DEF) {            
            char* labelName = instrs->args[0];

            Label* label = (Label*)litaMalloc(sizeof(Label));
            label->address = instrs->address;
            label->name = labelName;
            
            if(!program->labels) {
                program->labels = label;
                
            }
            else {
                current->next = label;
            }

            current = label;
            program->numberOfLabels++;
        }
        
        instrs = instrs->next;        
    }

}

static AssemblerInstruction* parseLine(size_t lineNumber, const char* line, const char* end) {    
    int inComment = 0;
    int inString  = 0;
    char* buf = NULL;
    
    // trim leading spaces
    for(; (*line == ' ' || *line == '\r') && line < end; line++);
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
        else if(c == '\r') {
            continue; // causes problems if not skipped
        }
        else if(c == '"') {
            if(inString) {
                inString = 0;                
            }
            else if(!inComment) {
                inString = 1;                
            }
        }
        else if(c == ' ') {
            if(!inString && !inComment) {   
                if(buf_len(buf) > 0) {
                    buf_push(buf, 0);                
                    buf_push(instr->args, buf);
                    instr->numberOfArgs++;                

                    buf = NULL;
                }
                continue;
            }
        }

        if(!inComment) {            
            buf_push(buf, c);            
        }
    }

    if(buf_len(buf) > 0) {
        buf_push(buf, 0);                    
        buf_push(instr->args, buf);
        instr->numberOfArgs++;      
    }

    buf_push(instr->args, 0);
    
    if(instr->numberOfArgs > 0) {
        char* arg1 = instr->args[0];
        //printf("Line: '%s' => len: '%zu'  at '%zu' \n", arg1, buf_len(arg1), lineNumber);
        if((buf_len(arg1) - 1) > 0) {
            char c = arg1[0];

            switch(c) {
                case '.': {
                    instr->kind = CONSTANT_DEF;
                    break;
                }
                case ':': {
                    instr->kind = LABEL_DEF;
                    break;
                }
                default: {
                    instr->kind = BYTECODE_DEF;
                }
            }

            return instr;
        }
    }
    
    freeAssemberInstruction(instr);
    return NULL;
}



void parse(Program* program, const char* assembly) {
    size_t lineNumber = 1;
    Address address = 0;
    AssemblerInstruction* current = NULL;

    const char* start = assembly;
    for(;;) {
        char c = *assembly;                
        if(c == '\n' || c == 0) {
            AssemblerInstruction* next = parseLine(lineNumber, start, assembly);            
            if(next) {                
                next->address = address;
                if(next->kind == BYTECODE_DEF) {
                    address++;                                        
                }
                
                if(program->instrs == NULL)  {
                    program->instrs = next;                    
                }
                else {
                    current->next = next;
                }

                current = next;
            }

            lineNumber++;
            start = assembly + 1;
        }
        
        if(c == 0) {
            break;
        }

        assembly++;
    } 

    program->numberOfInstructions = address;
}


Bytecode* compile(Vm* vm, const char* assembly) {
    Program program = {
        .instrs = NULL,
        .numberOfInstructions = 0,        
        .constants = NULL,
        .numberOfConstants = 0,
        .labels = NULL,
        .numberOfLabels = 0
    };

    parse(&program, assembly);        
    parseLabels(&program);

    Address* constants = parseConstants(vm, &program);
    Instruction* instructions = parseInstructions(&program);

    Bytecode* code = (Bytecode*)litaMalloc(sizeof(Bytecode));
    code->constants = constants;
    code->numOfConstants = program.numberOfConstants;
    code->instrs = instructions;
    code->length = program.numberOfInstructions;
    code->pc = 0;

    // TODO - remove AssemblerInstruction heap allocations
    // construct bytecode instructions per line parsing iteration        
    freeAssemberInstruction(program.instrs);    
    freeConstants(program.constants);
    freeLabels(program.labels);

    return code;
}

void      disassemble(Bytecode* code) {    
    for(size_t i = 0; i < code->length; i++) {
        Instruction instr = code->instrs[i];

        Opcode opcode = OPCODE(instr);
        printf("%-5zu   %s ", i, OpcodeStr[opcode]);
        switch(opcode) {
            case JMP:
            case CALL:
                printf("%d", ARG_JMP_VALUE(instr));
                break;
            default: {
                switch(opcodeNumArgs(opcode)) {
                    case 2:
                        if(IS_ARG1_ADDR(instr)) {
                            printf("&");
                        }

                        printf("%s ", RegisterNames[ARG1_VALUE(instr)]);
                        // fallthrough
                    case 1: {
                        switch(opcode) {
                            case LDCI:
                            case LDCB:
                                if(IS_ARG2_IMM(instr)) {
                                    printf("%d", ARG2_VALUE(instr));
                                }
                                else {
                                    printf("%d", code->constants[ARG2_VALUE(instr)]);
                                }
                                break;
                            case LDCF:
                            case LDCA:
                                printf("%d", code->constants[ARG2_VALUE(instr)]);
                                break;
                            default:
                                if(IS_ARG2_REG(instr)) {
                                    if(IS_ARG2_ADDR(instr)) {
                                        printf("&");
                                    }
                                    printf("%s", RegisterNames[ARG2_VALUE(instr)]);
                                }
                                else {
                                    if(IS_ARG2_IMM(instr)) {
                                        printf("#");
                                    }                                
                                    printf("%d", ARG2_VALUE(instr));
                                }
                        }

                        break;
                    }
                    case 0: break;                    
                }
            }
        }

        printf("\n");
    }
}
