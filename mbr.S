.code16
.begin:
.global _start
_start:
	cli
	jmp .loop
.space 0x80
.next:
	incb %cs:.drive
	cmpb $0, %cs:.drive
	je panic
.loop:
	mov $8, %ah # get CHS
	mov %cs:.drive, %dl
	int $0x13
	jc .next
	and $0x3F, %cl
	mov %cl, %cs:.sectors
	inc %dh
	mov %dh, %cs:.heads

	mov $0x07E0, %ax
	mov %ax, %es
	mov $0, %bx

	mov $1, %al # 1 sector
	mov %cs:.drive, %dl # drive
	mov $2, %cl # sector
	mov $0, %dh # head
	mov $0, %ch # cylinder
	mov $2, %ah # read
	int $0x13
	jc .next
	cmpw $0x2EFA, %es:0
	jne .next

	mov $1, %al # read 1 sector
	mov %cs:.drive, %dl
	mov $2, %cl # sector
	mov $0, %dh # head
	mov $0, %ch # cylinder
	
	mov $0, %si
.contread:
	cmp $500, %si
	je .endread

	mov $2, %ah # read
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
	mov %cs:.drive, %al
	jmp $0x0000, $0x7E00

halt:
	cli
	hlt
	jmp halt

panic:
	mov $0xB800, %ax
	mov %ax, %ds
	movw $'P'|0x0C00, 0
	movw $'a'|0x0C00, 2
	movw $'n'|0x0C00, 4
	movw $'i'|0x0C00, 6
	movw $'c'|0x0C00, 8
	jmp halt

.drive:
	.byte 0
.sectors:
	.byte 0
.heads:
	.byte 0

.space 0x1FE - (. - .begin)
.byte 0x55, 0xAA
