@echo off

del /S /Q "litavm.exe" >nul 2>&1
del /S /Q "*.ilk" >nul 2>&1
del /S /Q "*.pdb" >nul 2>&1

REM -Wconversion

clang -std=c11  -g ./src/main.c -o ./bin/litavm.exe -Wdeprecated-declarations -Wall -pedantic-errors -Wno-error=unused-variable -Wextra -Wunreachable-code -Wuninitialized 
cd bin/
litavm.exe %*