.extern csv, cret

csv:
    pop ax
    push bp
    mov bp, sp
    push bx
    push si
    push di
    sub sp, #2
    jmp (ax)

cret:
    add sp, #2
    pop di
    pop si
    pop bx
    pop bp
    ret
