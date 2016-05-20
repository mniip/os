.section .text
.code16
.global _start
_start:
	cli
	
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
	or $1, %eax
	mov %eax, %cr0

	jmp $0x10, $_start32

halt:
	cli
	hlt
	jmp halt

panic:
	mov %cs, %ax
	mov %ax, %ds
	mov $msg_panic, %si
	mov $0xB800, %ax
	mov %ax, %es
	mov $0, %di
	mov $5, %cx
	rep movsw
	jmp halt

msg_panic:
	.byte 'P', 0x0C, 'A', 0x0C, 'N', 0x0C, 'I', 0x0C, 'C', 0x0C

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
	.byte 0xFC
	# base 24:31
	.byte 0x00

	# limit 0:15
	.short 0xFFFF
	# base 0:15
	.short 0x0000
	# base 16:23
	.byte 0x00
	# access: present=1, privilege=0 executable=0 direction=0 rw=1
	.byte 0x9A
	# limit 16:19 + flags (granularity=1, size=1)
	.byte 0xFC
	# base 24:31
	.byte 0x00
gdt_end:

.code32
_start32:
	mov $0x08, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov $0x7C00, %esp

	call main
.loop:
	hlt
	jmp .loop

.section .data
.global mem_info_sz
.equ mem_info_sz, 0x500
.global mem_info
.equ mem_info, 0x504