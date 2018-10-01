[BITS 64]

SECTION .text

extern Main

START:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; 0x600000 ~ 0x6FFFFF (1MB stack)
	mov ss, ax
	mov rsp, 0x6FFFF8
	mov rbp, 0x6FFFF8

	call Main

	jmp $

