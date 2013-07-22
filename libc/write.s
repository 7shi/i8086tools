.extern _write
_write:
	push bp
	mov bp, sp
	mov ax, 6(bp)
	mov 0f+2, ax
	mov ax, 8(bp)
	mov 0f+4, ax
	mov ax, 4(bp)
	int 7
	.data1 0
	.data2 0f
	mov sp, bp
	pop bp
	ret

.sect .data
0:	.data2 4, 0, 0
