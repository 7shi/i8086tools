mov ax, 1
mov ah, 2
mov al, 3
mov bx, 4
mov bh, 5
mov bl, 6
push ax
mov bp, sp
mov cx, [bp]
mov [bp], bh
mov [bp+1], bl
pop di
ret
