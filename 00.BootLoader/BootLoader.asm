[ORG 0x00]		; 코드의 시작 어드레스를 0x00으로 설정
[BITS 16]		; 이하의 코드는 16비트 코드로 설정	

SECTION .text		; text 섹션을 정의


jmp 0x07C0:START	;CS <- 0x07C0 and jump to START

START:
	mov ax, 0x07C0		; AX 레지스터에 0x07C0 복사
	mov ds, ax		; copy AX to DS
	mov ax, 0xB800		
	mov es, ax	

	mov si, 0

.SCREENCLEAERLOOP:
	mov byte [es: si], 0

	mov byte [es: si + 1], 0x0A0

	add si, 2

	cmp si, 80 * 25 * 2

	jl .SCREENCLEAERLOOP

	mov si, 0
	mov di, 0

.MESSAGELOOP:
	mov cl, byte [si + MESSAGE1]

	cmp cl, 0
	je .MESSAGEEND


	mov byte [es: di], cl

	add si, 1
	add di, 2

	jmp .MESSAGELOOP
.MESSAGEEND

jmp $

MESSAGE1: db 'MINT64 OS Boot Loader Start~!!', 0

times 510 - ($ - $$) db 0x00

db 0x55
db 0xAA
