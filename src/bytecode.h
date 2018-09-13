#ifndef BYTECODE_H
#define BYTECODE_H

typedef int32_t Instruction;
typedef uint32_t Address;

// 0b11_1111_1111_1111_1111_1
#define MAX_IMMEDIATE_VALUE 0x7ffff

    
#define INSTRUCTION_SIZE 32
#define OPCODE_SIZE 6
#define OPCODE_SHIFT ((INSTRUCTION_SIZE) - (OPCODE_SIZE))

// 0b111111
#define OPCODE_MASK 0x3f
#define ARG1_SIZE 5
#define ARG1_SHIFT 21
// 0b11111
#define ARG1_MASK 0x1f
// 0b10000
#define ARG1_ADDR_MASK 0x10
// 0b01111
#define ARG1_VALUE_MASK 0xf

#define ARG2_SIZE 21
#define ARG2_SHIFT 0
//  0b1111_1111_1111_1111_1111_1
#define ARG2_MASK 0x1fffff
// 0b1000_0000_0000_0000_0000_0
#define ARG2_REG_MASK 0x100000
// 0b0100_0000_0000_0000_0000_0
#define ARG2_ADDR_MASK 0x80000
// 0b0100_0000_0000_0000_0000_0
#define ARG2_IMM_MASK 0x80000
// 0b0011_1111_1111_1111_1111_1
#define ARG2_VALUE_MASK 0x7ffff    
// 0b000000_1111_1111_1111_1111_1111_1111    
#define ARG_JMP_VALUE_MASK 0xffffff

#define OPCODE(instruction) (instruction >> OPCODE_SHIFT)

#define IS_ARG1_ADDR(instruction) (((instruction >> ARG1_SHIFT) & ARG1_ADDR_MASK) != 0)
#define ARG1_VALUE(instruction) ((instruction >> ARG1_SHIFT) & ARG1_VALUE_MASK)

#define IS_ARG2_REG(instruction) (((instruction >> ARG2_SHIFT) & ARG2_REG_MASK) != 0)
#define IS_ARG2_ADDR(instruction) (((instruction >> ARG2_SHIFT) & ARG2_ADDR_MASK) != 0)
#define IS_ARG2_IMM(instruction) (((instruction >> ARG2_SHIFT) & ARG2_IMM_MASK) != 0)
#define ARG2_VALUE(instruction) ((instruction >> ARG2_SHIFT) & ARG2_VALUE_MASK)

#define ARG_JMP_VALUE(instruction) (instruction & ARG_JMP_VALUE_MASK)

typedef enum Opcode {
    NOOP,
    MOVI,   // Moves the int value to the first register MOVI $a $b ($a = $b)
    MOVF,   // Moves the float value to the first register MOVF $a $b ($a = $b)
    MOVB,   // Moves the byte value to the first register MOVB $a $b ($a = $b)
    
    LDCI,   // Loads a int constant into a register LDCI $a
    LDCF,   // Loads a float constant into a register LDCF $a
    LDCB,   // Loads a byte of constant into a register LDCB $a
    LDCA,   // Loads the address of the constant into a register LDCA $a
    
    PUSHI,   // PUSH's the register value onto the stack PUSHI $a
    PUSHF,   // PUSH's the register value onto the stack PUSHF $a
    PUSHB,   // PUSH's the register value onto the stack PUSHB $a
    
    POPI,  // POP's the top of the stack into the register POPI $a
    POPF,  // POP's the top of the stack into the register POPF $a
    POPB,  // POP's the top of the stack into the register POPB $a
    
    DUPI,  // Duplicates the top of the stack, stores the top in register $a
    DUPF,  // Duplicates the top of the stack, stores the top in register $a
    DUPB,  // Duplicates the top of the stack, stores the top in register $a
    
    IFI,  // If (integer) $a > $b skips the next instruction; IFI $a $b
    IFF,  // If (float)   $a > $b skips the next instruction; IFF $a $b
    IFB,  // If (byte)    $a > $b skips the next instruction; IFB $a $b
    
    IFEI,  // If (integer) $a >= $b skips the next instruction; IFEI $a $b
    IFEF,  // If (float)   $a >= $b skips the next instruction; IFEF $a $b
    IFEB,  // If (byte)    $a >= $b skips the next instruction; IFEB $a $b
    
    JMP,   // Jumps the amount of $b
    
    PRINTI,  // Prints registers
    PRINTF,  // Prints registers
    PRINTB,  // Prints the byte at the supplied address
    PRINTC,  // Prints the byte (as a character) at the supplied address
    
    CALL,  // CALL $b  pushes the address on the next instruction onto the stack
    RET,   // stores return value in $x registers and moves the PC to the the value on the top of the stack 
    
    ADDI,  // Adds two integers ADDI $a $b => $a = $a + $b
    ADDF,  // Adds two floats ADDF $a $b => $a = $a + $b
    ADDB,  // Adds two bytes ADDB $a $b => $a = $a + $b
    
    SUBI,  // Subtracts two integers SUBI $a $b => $a = $a - $b
    SUBF,  // Subtracts two floats SUBF $a $b => $a = $a - $b
    SUBB,  // Subtracts two bytes SUBB $a $b => $a = $a - $b
    
    MULI,  // Multiplies two integers MULI $a $b => $a = $a * $b
    MULF,  // Multiplies two floats MULF $a $b => $a = $a * $b
    MULB,  // Multiplies two bytes MULB $a $b => $a = $a * $b
    
    DIVI,  // Divides two integers DIVI $a $b => $a = $a / $b
    DIVF,  // Divides two floats DIVF $a $b => $a = $a / $b
    DIVB,  // Divides two bytes DIVB $a $b => $a = $a / $b
    
    MODI,  // Remainder of two integers MODI $a $b => $a = $a % $b
    MODF,  // Remainder of two floats MODI $a $b => $a = $a % $b
    MODB,  // Remainder of two bytes MODI $a $b => $a = $a % $b
    
    ORI,   // Bitwise OR of two integers ORI $a $b => $a = $a | $b
    ORB,   // Bitwise OR of two bytes ORB $a $b => $a = $a | $b
    
    ANDI,  // Bitwise AND of two integers ANDI $a $b => $a = $a & $b
    ANDB,  // Bitwise AND of two bytes ANDB $a $b => $a = $a & $b
    
    NOTI,  // Bitwise NOT of the integer NOTI $a $b => $a = ~$b
    NOTB,  // Bitwise NOT of the byte NOTB $a $b => $a = ~$b
    
    XORI,  // Bitwise exclusive OR of the integers XORI $a $b => $a = $a ^ $b
    XORB,  // Bitwise exclusive OR of the bytes XORI $a $b => $a = $a ^ $b
    
    SZRLI, // Bitwise shift zero right logical operator for integer SZRLI $a $b => $a >>> $b
    SZRLB, // Bitwise shift zero right logical operator for byte SZRLB $a $b => $a >>> $b
    
    SRLI,  // Bitwise shift right logical operator for integer SRLI $a $b => $a >> $b
    SRLB,  // Bitwise shift right logical operator for byte SRLB $a $b => $a >> $b
    
    SLLI,  // Bitwise shift left logical operator for integer SLLI $a $b => $a << $b
    SLLB,  // Bitwise shift left logical operator for byte SLLB $a $b => $a << $b

    MAX_OPCODES
} Opcode;

const char* OpcodeStr[] = {
    [NOOP] = "NOOP",
    [MOVI] = "MOVI",   
    [MOVF] = "MOVF",   
    [MOVB] = "MOVB",   
    
    [LDCI] = "LDCI",   
    [LDCF] = "LDCF",   
    [LDCB] = "LDCB",   
    [LDCA] = "LDCA",   
    
    [PUSHI] = "PUSHI", 
    [PUSHF] = "PUSHF", 
    [PUSHB] = "PUSHB", 
    
    [POPI] = "POPI",  
    [POPF] = "POPF",  
    [POPB] = "POPB",  
    
    [DUPI] = "DUPI",  
    [DUPF] = "DUPF",  
    [DUPB] = "DUPB",  
    
    [IFI] = "IFI",  
    [IFF] = "IFF",  
    [IFB] = "IFB",  
    
    [IFEI] = "IFEI",
    [IFEF] = "IFEF",
    [IFEB] = "IFEB",
    
    [JMP] = "JMP",   
    
    [PRINTI] = "PRINTI", 
    [PRINTF] = "PRINTF", 
    [PRINTB] = "PRINTB", 
    [PRINTC] = "PRINTC", 
    
    [CALL] = "CALL", 
    [RET] = "RET",   
    
    [ADDI] = "ADDI", 
    [ADDF] = "ADDF", 
    [ADDB] = "ADDB", 
    
    [SUBI] = "SUBI", 
    [SUBF] = "SUBF", 
    [SUBB] = "SUBB", 
    
    [MULI] = "MULI", 
    [MULF] = "MULF", 
    [MULB] = "MULB", 
    
    [DIVI] = "DIVI", 
    [DIVF] = "DIVF", 
    [DIVB] = "DIVB", 
    
    [MODI] = "MODI", 
    [MODF] = "MODF", 
    [MODB] = "MODB", 
    
    [ORI] = "ORI",   
    [ORB] = "ORB",   
    
    [ANDI] = "ANDI",  
    [ANDB] = "ANDB",  
    
    [NOTI] = "NOTI",  
    [NOTB] = "NOTB",  
    
    [XORI] = "XORI",  
    [XORB] = "XORB",  
    
    [SZRLI] = "SZRLI",
    [SZRLB] = "SZRLB",
    
    [SRLI] = "SRLI", 
    [SRLB] = "SRLB", 
    
    [SLLI] = "SLLI", 
    [SLLB] = "SLLB"  
};

Opcode opcodeFromString(const char* opcodeStr);
size_t opcodeNumArgs(Opcode opcode);

typedef struct Bytecode {
    Address* constants;
    size_t   numOfConstants;
    
    Instruction* instrs;
    Address length;
    Address pc;

} Bytecode;

void bytecodeFree(Bytecode* code);

#endif
