@echo off
REM tcc -o litavm.exe ./src/common.c ./src/buf.c ./src/parser.c ./src/assembler.c -run ./src/main.c

REM ./src/common.c ./src/buf.c ./src/parser.c ./src/assembler.c
clang -std=c11  ./src/main.c -o ./bin/litavm.exe -Wdeprecated-declarations
cd bin/
litavm.exe