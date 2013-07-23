.extern .cret, .sret, .dsret
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
