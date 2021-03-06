.section .start
.code16
.global _start
_start:
	cli
	mov %al, %cs:drive

	# load gdt
	lgdt %cs:gdt

	# enable A20
	in $0x92, %al
	or $2, %al
	out %al, $0x92
	
	# detect RAM
	xor %ebx, %ebx
	mov $0x534D4150, %edx
	mov $mem_info_sz/16, %ax
	mov %ax, %es
	mov $mem_info-mem_info_sz, %di
.detectmore:
	mov $0xE820, %eax
	mov $24, %ecx
	int $0x15
	jc panic
	add $24, %di
	incl %es:0
	test %ebx, %ebx
	jnz .detectmore

	# enable protected mode
	mov %cr0, %eax
	or $0x1, %eax
	mov %eax, %cr0

	jmp $0x0010, $_start32

.global halt
halt:
	cli
	hlt
	jmp halt

.global drive
drive:
	.byte 0

panic:
	mov $0xB800, %ax
	mov %ax, %ds
	movw $'P'|0x0E00, 0
	movw $'a'|0x0E00, 2
	movw $'n'|0x0E00, 4
	movw $'i'|0x0E00, 6
	movw $'c'|0x0E00, 8
	jmp halt

gdt:
	# size
	.short gdt_end - gdt_table - 1
	# offset
	.int gdt_table
gdt_table:
	# null descriptor
	.int 0, 0

	# limit 0:15
	.short 0xFFFF
	# base 0:15
	.short 0x0000
	# base 16:23
	.byte 0x00
	# access: present=1, privilege=0 executable=0 direction=0 rw=1
	.byte 0x92
	# limit 16:19 + flags (granularity=1, size=1)
	.byte 0xCF
	# base 24:31
	.byte 0x00

	# limit 0:15
	.short 0xFFFF
	# base 0:15
	.short 0x0000
	# base 16:23
	.byte 0x00
	# access: present=1, privilege=0 executable=1 direction=0 rw=1
	.byte 0x9A
	# limit 16:19 + flags (granularity=1, size=1)
	.byte 0xCF
	# base 24:31
	.byte 0x00

	# limit 0:15
	.short 0xFFFF
	# base 0:15
	.short 0x0000
	# base 16:23
	.byte 0x00
	# access: present=1, privilege=0 executable=1 direction=0 rw=1
	.byte 0x9A
	# limit 16:19 + flags (granularity=1, size=0)
	.byte 0x8F
	# base 24:31
	.byte 0x00
gdt_end:

.code32
_start32:
	cli
	mov $0x0008, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov $0x7C00, %esp
	pushl $0
	mov %esp, %ebp

	mov %cr0, %eax
	and $0xFFFFFFFB, %eax
	or $0x2, %eax
	mov %eax, %cr0
	mov %cr4, %eax
	or $0x600, %eax
	mov %eax, %cr4
	fninit

	call main
.loop:
	hlt
	jmp .loop

.section .data
.global mem_info_sz
.equ mem_info_sz, 0x500
.global mem_info
.equ mem_info, 0x504

.section .text
idt:
	.short 0x3FF
	.int 0x00000000
.code16
.global _int13
_int13:
	cli
	lidt idt
	push %ax
	mov %cr0, %eax
	and $0xFFFFFFFE, %eax
	mov %eax, %cr0
	jmp $0x0000, $_int13_real
_int13_real:
	cli
	xor %ax, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	pop %ax
	int $0x13
	push %ax
	mov %cr0, %eax
	or $0x1, %eax
	mov %eax, %cr0
	jmp $0x0010, $_int13_protected
.code32
_int13_protected:
	cli
	mov $0x0008, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	pop %ax
	lret
