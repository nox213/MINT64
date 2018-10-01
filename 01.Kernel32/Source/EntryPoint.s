[ORG 0x00]
[BITS 16]

SECTION .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
	mov ax, 0x1000 ; Entry Point is in 0x10000
	mov ds, ax
	mov es, ax
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Activating A20 Gate
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ax, 0x2401
	int 0x15

	jc .A20GATEERROR
	jmp .A20GATESUCCESS

.A20GATEERROR:
	; 에러 발생 시, 시스템 컨트롤 포트로 전환 시도
	in al, 0x92	; read from system port
	or al, 0x02	; setting A20 Gate bit(bit 1) to 1
	and al, 0xFE ; prevent system reset by setting first bit to 0
	out 0x92, al

.A20GATESUCCESS:
	cli
	lgdt [GTDR]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 보호 모드로 진입
	; Disable Paging, Disable Cache, Internal FPU, Disable align Check,
	; Enable ProtectedMode
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, 0x4000003B ; PG = 0, CD = 1, NW = 0, AM = 0, WP = 0, 
						; NE = 1, ET = 1, TS = 1, EM = 0, MP = 1, PE = 1
	mov cr0, eax
	

	; 커널 코드 세그먼트를 0x00을 기준으로 하는 것으로 교체하고 EIP의 값을 0x00 기준으로 재설정
	; CS 세그먼트 셀렉터 : EIP
	jmp dword 0x18 : (PROTECTEDMODE - $$ + 0x10000)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; 보호 모드로 진입
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]
PROTECTEDMODE:
	mov ax, 0x20
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; 스택을 0x00000000 ~ 0x0000FFFF 영역에 64KB 크기로 생성
	mov ss, ax
	mov esp, 0xFFFE
	mov ebp, 0xFFFE

	; 화면에 보호 모드로 전환되었다는 메시지 출력
	push (SWITCHSUCCESSMESSAGE - $$ + 0x10000)
	push 2
	push 0
	call PRINTMESSAGE
	add esp, 12

	jmp dword 0x18 : 0x10200

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 함수 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PRINTMESSAGE:
	push ebp
	mov ebp, esp
	push esi
	push edi
	push eax
	push ecx
	push edx

	; calculate Y coordinate
	mov eax, dword [ebp + 12]
	mov esi, 160
	mul esi
	mov edi, eax

	; calculate X coordinate
	mov eax, dword [ebp + 8]
	mov esi, 2
	mul esi
	add edi, eax

	; address of the string
	mov esi, dword [ebp + 16]
	
.MESSAGELOOP:
	mov cl, byte [esi]
	cmp cl, 0
	je .MESSAGEEND

	mov byte [edi + 0xb8000], cl

	add esi, 1
	add edi, 2
	jmp .MESSAGELOOP

.MESSAGEEND:
	pop edx
	pop ecx
	pop eax
	pop edi
	pop esi
	pop ebp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 데이터 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 아래의 데이터들을 8바이트에 맞춰서 정력하기 위해 추가
align 8, db 0

dw 0x0000

GTDR:
	dw GDTEND - GDT - 1
	dd (GDT - $$ + 0x10000)
	
GDT:
	NULLDESCRIPTOR:
		dw 0x0000
		dw 0x0000
		db 0x00
		db 0x00
		db 0x00
		db 0x00

	IA_32eCODEDESCRIPTOR:
		dw 0xFFFF	; Limit [15 : 0]
		dw 0x0000	; Base [15 : 0]
		db 0x00		; Base [23 : 16] 
		db 0x9A		; P = 1, DPL = 0, Code Segment, Execute/Read
		db 0xAF		; G = 1, D = 0, L = 1, Limit[19 : 16]
		db 0x00		; Base [31 : 24]

	IA_32eDATAEDESCRIPTOR:
		dw 0xFFFF	; Limit [15 : 0]
		dw 0x0000	; Base [15 : 0]
		db 0x00		; Base [23 : 16] 
		db 0x92		; P = 1, DPL = 0, Data Segment, Read/Write
		db 0xAF		; G = 1, D = 0, L = 1, Limit[19 : 16]
		db 0x00		; Base [31 : 24]

	CODEDESCRIPTOR:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0xCF
		db 0x00

	DATADESCRIPTOR:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0xCF
		db 0x00
GDTEND:

SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success~!!', 0

times 512 - ($ - $$) db 0x00
