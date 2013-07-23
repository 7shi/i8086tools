start:
	pop ax
	push sp
	push ax
	call _main
	push ax
	call _exit
