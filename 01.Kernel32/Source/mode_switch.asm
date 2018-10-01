[BITS 32]

global read_cpuid, switch_and_execute_64bit_kernel

SECTION .text

; return cpuid
; parameter: eax, DWORD *eax, DWORD *ebx, DWORD *ecx, DWORD *edx
read_cpuid:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx
	push esi

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; EAX 레지스터의 값으로 CPUID 명령어 실행
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, dword [ebp + 8]
	cpuid

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 반환된 값을 파라미터에 저장
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov esi, dword [ebp + 12]
	mov dword [esi], eax

	mov esi, dword[ebp + 16]
	mov dword [esi], ebx

	mov esi, dword[ebp + 20]
	mov dword [esi], ecx

	mov esi, dword[ebp + 24]
	mov dword [esi], edx

	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	pop ebp

	ret

; switch mode to IA-32e and execute 64bit kernel
; no parmater
switch_and_execute_64bit_kernel:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; CR4 레지스터의 PAE 비트를 1로 설정
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, cr4
	or eax, 0x20
	mov cr4, eax
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; CR3 레지스터의 PML4 테이블의 어드레스와 캐시 활성화
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, 0x100000
	mov cr3, eax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; set IA23_EFER.LME to 1 (enable IA-32e mode)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ecx, 0xC0000080
	rdmsr

	or eax, 0x0100

	wrmsr

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; set NW bit and CD bit to 0, set PG bit to 1
	; (enable cache and paging)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, cr0
	or eax, 0xE0000000
	xor eax, 0x60000000
	mov cr0, eax

	jmp 0x08:0x200000

	; never reach
	jmp $

