.section .text
.global int13_08
int13_08:
	push %ebp
	mov %esp, %ebp
	mov $8, %ah
	mov 8(%ebp), %dl
	pusha
	push %ebp
	sidt idt
	lcall $0x0018, $_int13
	cli
	lidt idt
	setc %al
	pop %ebp
	mov 12(%ebp), %ebx
	mov %ah, (%ebx)
	mov 16(%ebp), %ebx
	mov %dh, (%ebx)
	mov 20(%ebp), %ebx
	mov %cx, (%ebx)
	test %al, %al
	popa
	sete %al
	movzbl %al, %eax
	leave
	ret

.global int13_02
int13_02:
	push %ebp
	mov %esp, %ebp
	mov $2, %ah
	mov 8(%ebp), %dl
	mov 12(%ebp), %dh
	mov 16(%ebp), %cx
	mov 20(%ebp), %al
	mov 24(%ebp), %bx
	pusha
	push %ebp
	sidt idt
	lcall $0x0018, $_int13
	cli
	lidt idt
	setc %cl
	pop %ebp
	mov 28(%ebp), %ebx
	mov %ah, (%ebx)
	mov 32(%ebp), %ebx
	mov %al, (%ebx)
	test %cl, %cl
	popa
	sete %al
	movzbl %al, %eax
	leave
	ret

.global int13_03
int13_03:
	push %ebp
	mov %esp, %ebp
	mov $3, %ah
	mov 8(%ebp), %dl
	mov 12(%ebp), %dh
	mov 16(%ebp), %cx
	mov 20(%ebp), %al
	mov 24(%ebp), %bx
	pusha
	push %ebp
	sidt idt
	lcall $0x0018, $_int13
	cli
	lidt idt
	setc %cl
	pop %ebp
	mov 24(%ebp), %ebx
	mov %ah, (%ebx)
	mov 28(%ebp), %ebx
	mov %al, (%ebx)
	test %cl, %cl
	popa
	sete %al
	movzbl %al, %eax
	leave
	ret

.section .data
idt:
	.space 6
