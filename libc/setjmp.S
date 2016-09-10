.global setjmp
setjmp:
	mov 4(%esp), %eax
	mov %ebx, 0(%eax)
	mov %esi, 4(%eax)
	mov %edi, 8(%eax)
	mov %ebp, 12(%eax)
	mov %esp, 16(%eax)
	mov (%esp), %ecx
	mov %ecx, 20(%eax)
	mov $0, %eax
	ret

.global longjmp
longjmp:
	mov 4(%esp), %ecx
	mov 8(%esp), %eax
	mov 16(%ecx), %esp
	mov 20(%ecx), %ebx
	mov %ebx, (%esp)
	mov 12(%ecx), %ebp
	mov 8(%ecx), %edi
	mov 4(%ecx), %esi
	mov 0(%ecx), %ebx
	ret
