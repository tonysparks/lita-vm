@echo off

del /S /Q "litavm.exe"
del /S /Q "*.ilk"
del /S /Q "*.pdb"

clang -std=c11  -g ./src/main.c -o ./bin/litavm.exe -Wdeprecated-declarations
cd bin/
litavm.exe %*