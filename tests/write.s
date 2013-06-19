sub sp, #20
mov bx, sp

mov  2(bx), #4
mov  4(bx), #1
mov 10(bx), #hello
mov  6(bx), #6
int 0x20

mov  2(bx), #1
mov  4(bx), #0
int 0x20

.sect .data
hello: .ascii "hello\n"
