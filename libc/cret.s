.extern .cret, .sret, .dsret, .csb2
.cret:
	mov sp, bp
	pop bp
	ret

.sret:
	lea sp, -2(bp)
	pop si
	pop bp
	ret

.dsret:
	lea sp, -4(bp)
	pop di
	pop si
	pop bp
	ret

.csb2:
	mov dx, (bx)
	mov cx, 2(bx)
0:	add bx, #4
	cmp ax, (bx)
	jnz 1f
	mov dx, 2(bx)
	jmp (dx)
1:	loop 0b
	jmp (dx)
