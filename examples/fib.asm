;; calculate fibonacci number
pushi #2991   ; pick a big number
call :fib
popi $a
printi $a
printc #10

jmp :exit

;;
;; calculates fibonacci sequence
;;
;; input:
;;    $sp <int>
;; output:
;;    $sp <int>
;;     
:fib        
        popi $a    ; take top of stack as the input; the number of fib sequences we should run                
        movi $j #0 ; n1
        movi $k #1 ; n2       
        
        ; If N >= 2 return N
        ifei $a #2
        jmp :fib_end_loop
                
        
        movi $i #3 ; iteration index
    :fib_loop        
        ifei $a $i    ; if N >= i return $c
        jmp :fib_end_loop
                
        ; sum = n1 + n2
        ; n1 = n2
        ; n2 = sum
        movi $b #0
        addi $b $j
        addi $b $k
        movi $j $k
        movi $k $b
                
        addi $i #1   ; i++
        jmp :fib_loop
    :fib_end_loop
    
        movi $c #0   ; clear out $c
        addi $c $j
        addi $c $k
        
        pushi $c     ; return n1+n2                
        ret

:exit
