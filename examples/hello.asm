.text "Hello World"

ldca $a .text
pushi $a             ;; push the address on the stack, so that print_string can use it
call :print_string   ;; call the print_string subroutine

jmp :exit            ; exit out of the program
printi #11           ; shouldn't get invoked, because we are jumping to the :exit label

;;
;; Prints the supplied string to sysout
;;
;; input:
;;    <address> to string constant, retreived from top of the stack
;; output:
;;    <void>
;;
:print_string        
        popi $a          ;; stores the address of the string constant
    :print_loop
        ifb &$a #0       ;; loops until the value at address $a = 0; strings are null terminated
        jmp :print_end_loop
        printc &$a       ;; prints out the ASCII byte character
        addi $a #1       ;; increments to the next character byte
        jmp :print_loop
    :print_end_loop    
        ret              ;; Stores return $pc in $r register, RET sets the $pc to value of $r
       
:exit