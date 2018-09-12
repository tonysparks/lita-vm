.hex 0xffff
.bin 0b1100
.bin2 255

ldci $a .hex
printi $a
printc #10

printi .hex  ;; 
printc #10

printb .hex  ;; truncated, because Constant is Int32, but reading Int8
printc #10


printi .bin  ;; should collide, with next constant value (since it is reading Int32)
printc #10


printb .bin ;; should not collide, proper size of Int8
printc #10