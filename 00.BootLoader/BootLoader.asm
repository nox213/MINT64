[ORG 0x00]			; 코드의 시작 어드레스를 0x00으로 설정
[BITS 16]			; 이하의 코드는 16비트 코드로 설정	

SECTION .text		; text 섹션을 정의

jmp 0x07C0:START	;CS <- 0x07C0 and jump to START

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  MINT64 OS에 관련된 환경 설정 값
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TOTALSECTORCOUNT: dw 2		; 부트 로더를 제외한 MINT64 OS 이미지의 크기
								; 최대 1152 섹터(0x90000byte)까지 가능

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
	mov ax, 0x07C0	; AX 레지스터에 0x07C0 복사
	mov ds, ax		; copy AX to DS
	mov ax, 0xB800		
	mov es, ax	

	; 스택을 0x0000:0000~0x:0000:FFFF 영역에 64KB 크기로 생성
	mov ax, 0x0000	
	mov ss, ax;
	mov sp, 0xFFFE
	mov bp, 0xFFFE

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 화면을 모두 지우고, 속성값을 녹색으로 설정
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov si, 0

.SCREENCLEAERLOOP:
	mov byte [es: si], 0

	mov byte [es: si + 1], 0x0A

	add si, 2

	cmp si, 80 * 25 * 2

	jl .SCREENCLEAERLOOP

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 화면 상단에 시작 메시지 출력
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push MESSAGE1
	push 0
	push 0
	call PRINTMESSAGE
	add sp, 6

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; OS 이미지를 로딩한다는 메시지 출력 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push IMAGELOADINGMESSAGE
	push 1
	push 0
	call PRINTMESSAGE
	add sp, 6

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 디스크에서 OS 이미지를 로딩 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 디스크를 읽기 전에 먼저 리셋
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

RESETDISK:				;디스크 리셋 코드 시작
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;  BIOS Reset function 호출 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 서비스 번호 0, 드라이브 번호(0=Floppy)
	mov ax, 0
	mov dl, 0
	int 0x13
	; error handling
	jc HANDLEDISKERROR

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; read a sector from the disk
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; setting a destination 
	mov si, 0x1000
	mov es, si
	mov bx, 0x0000

	mov di, word [TOTALSECTORCOUNT] ; 복사할 OS 이미지의 섹터 수를 저장

READDATA:
	; 모든 섹터를 다 읽었는지 확인
	cmp di, 0
	je READEND
	sub di, 0x01

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;  BIOS Read function 호출 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ah, 0x02
	mov al, 0x01
	mov ch, byte [TRACKNUMBER]
	mov cl, byte [SECTORNUMBER]
	mov dh, byte [HEADNUMBER]
	mov dl, 0x00					; source drive number
	int 0x13
	jc HANDLEDISKERROR

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 복사할 어드레스와 트랙, 헤드, 섹터 어드레스 계산
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	add si, 0x0020	;읽은 바이트(512) 수만큼 주소 값 증가
	mov es, si

	mov al, byte[SECTORNUMBER]
	add al, 0x01
	mov byte [SECTORNUMBER], al
	cmp al, 19
	jl READDATA

	; 마지막 섹터까지 읽은 경우 헤드를 토글 함, 섹터 번호는 다시 1로
	xor byte [HEADNUMBER], 0x01
	mov byte [SECTORNUMBER], 0x01

	; 헤드 양면을 다 읽은 경우(1 -> 0) 트랙 번호를 증가 시킴
	cmp byte[HEADNUMBER], 0x00
	jne READDATA

	add byte [TRACKNUMBER], 0x01
	jmp READDATA

READEND: 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; OS 이미지가 완료되었다는 메시지를 출력
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push LOADINGCOMPLETEMESSAGE
	push 1
	push 20
	call PRINTMESSAGE
	add sp, 6

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 로딩한 가상 OS 이미지 실행
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	jmp 0x1000:0x0000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 함수 코드 영역 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; disk error handler
HANDLEDISKERROR:
	push DISKERRORMESSAGE
	push 1
	push 20
	call PRINTMESSAGE

	jmp $

; print function
; parameter: x coordinate, y coordinate, null-terminated string 
PRINTMESSAGE:
	push bp
	mov bp, sp

	push es
	push si
	push di
	push ax
	push cx
	push dx

	mov ax, 0xB800
	mov es, ax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; X, Y의 좌표로 비디오 메모리의 어드레스를 계산함
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; using Y coordinate
	mov ax, word [bp + 6]
	mov si, 160
	mul si
	mov di, ax

	; using X coordinate
	mov ax, word [bp + 4]
	mov si, 2
	mul si
	add di, ax


	;address of the string
	mov si, word [bp + 8]

.MESSAGELOOP:
	mov cl, byte [si]
	cmp cl, 0
	je .MESSAGEEND

	mov byte [es : di], cl

	add si, 1
	add di, 2

	jmp .MESSAGELOOP

.MESSAGEEND
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 데이터 영역 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MESSAGE1: db 'MINT64 OS Boot Loader Start~!!', 0
COUNT: db 'read', 0

DISKERRORMESSAGE: 		db 'DISK Error~!!', 0
IMAGELOADINGMESSAGE: 	db 'OS Image Loading...', 0
LOADINGCOMPLETEMESSAGE: db 'Complete~!!', 0

SECTORNUMBER: 			db 0x02
HEADNUMBER: 			db 0x00
TRACKNUMBER: 			db 0x00

times 510 - ($ - $$) db 0x00

db 0x55
db 0xAA
