[bits 16]

KERNEL_OFFSET equ 0x1000

mov [BOOT_DRIVE], dl

mov bp, 0x9000
mov sp, bp

call load_kernel
call switch_32bit

jmp $

load_kernel:
	mov bx, KERNEL_OFFSET
	mov dh, 30
	mov dl, [BOOT_DRIVE]
	call disk_load
	ret

[bits 32]
BEGIN_32BIT:
	call KERNEL_OFFSET
	jmp $

BOOT_DRIVE db 0

; DISK
[bits 16]
disk_load:
	pusha
	push dx

	xor ax, ax
	mov es, ax

	mov ah, 0x02
	mov al, dh
	mov cl, 0x02

	mov ch, 0x00
	mov dh, 0x00

	int 0x13
	jc disk_error

	pop dx
	cmp al, dh

	jne sectors_error
	popa
	ret

disk_error:
	jmp disk_loop

sectors_error:
	jmp disk_loop

disk_loop:
	jmp $

; GDT
gdt_start:
	dq 0x0

gdt_code:
	dw 0xffff
	dw 0x0
	db 0x0
	db 10011010b
	db 11001111b
	db 0x0

gdt_data:
	dw 0xffff
	dw 0x0
	db 0x0
	db 10010010b
	db 11001111b
	db 0x0

gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; SWITCH 32 BIT
[bits 16]
switch_32bit:
	cli
	lgdt [gdt_descriptor]
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax
	jmp CODE_SEG:init_32bit

[extern main]

[bits 32]
init_32bit:
	mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ebp, 0x90000
	mov esp, ebp

	call main
	jmp $

section .bootsig
db 0x55, 0xAA
