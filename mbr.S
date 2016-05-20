.code16
.begin:
.global _start
_start:
	cli

	mov $8, %ah # get CHS
	mov $0, %dl # drive A
	int $0x13
	and $0x3F, %cl
	mov %cl, %cs:.sectors
	inc %dh
	mov %dh, %cs:.heads

	mov $0x07E0, %ax
	mov %ax, %es
	mov $0, %bx

	mov $1, %al # 1 sector
	mov $0, %dl # drive 0

	mov $2, %cl # sector
	mov $0, %dh # head
	mov $0, %ch # cylinder
	
	mov $0, %si
.contread:
	cmp $30, %si
	je .endread

	mov $2, %ah # read
	clc
	int $0x13
	jc panic

	mov %es, %di
	add $0x20, %di
	mov %di, %es
	
	cmp %cl, %cs:.sectors
	jne .goodsector
	mov $0, %cl
	inc %dh
	cmp %dh, %cs:.heads
	jne .goodsector
	mov $0, %dh
	inc %ch
.goodsector:
	inc %cl

	inc %si
	jmp .contread
.endread:
	jmp $0x0000, $0x7E00

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

.sectors:
	.byte 0
.heads:
	.byte 0

.space 0x1FE - (. - .begin)
.byte 0xAA, 0x55
