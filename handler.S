.section .text
.global handler_UNK
handler_UNK:
	pushl $msg_UNK
	jmp handler

.global handler_DE
handler_DE:
	pushl $0
	pushl $msg_DE
	jmp handler

.global handler_DB
handler_DB:
	pushl $0
	pushl $msg_DB
	jmp handler

.global handler_NMI
handler_NMI:
	pushl $0
	pushl $msg_NMI
	jmp handler

.global handler_BP
handler_BP:
	pushl $0
	pushl $msg_BP
	jmp handler

.global handler_OF
handler_OF:
	pushl $0
	pushl $msg_OF
	jmp handler

.global handler_BR
handler_BR:
	pushl $0
	pushl $msg_BR
	jmp handler

.global handler_UD
handler_UD:
	pushl $0
	pushl $msg_UD
	jmp handler

.global handler_NM
handler_NM:
	pushl $0
	pushl $msg_NM
	jmp handler

.global handler_DF
handler_DF:
	pushl $msg_DF
	jmp handler

.global handler_TS
handler_TS:
	pushl $msg_TS
	jmp handler

.global handler_NP
handler_NP:
	pushl $msg_NP
	jmp handler

.global handler_SS
handler_SS:
	pushl $msg_SS
	jmp handler

.global handler_GP
handler_GP:
	pushl $msg_GP
	jmp handler

.global handler_PF
handler_PF:
	pushl $msg_PF
	jmp handler

.global handler_MF
handler_MF:
	pushl $0
	pushl $msg_MF
	jmp handler

.global handler_AC
handler_AC:
	pushl $msg_AC
	jmp handler

.global handler_MC
handler_MC:
	pushl $0
	pushl $msg_MC
	jmp handler

.global handler_XM
handler_XM:
	pushl $0
	pushl $msg_XM
	jmp handler

.global handler_VE
handler_VE:
	pushl $0
	pushl $msg_VE
	jmp handler

.global handler_SX
handler_SX:
	pushl $msg_SX
	jmp handler

handler:
	pushal
	movzwl 44(%esp), %eax
	pushl %eax
	pushl 44(%esp) 
	pushl 8(%esp) 
	pushl 32(%esp) 
	pushl 20(%esp) 
	pushl 44(%esp) 
	pushl 32(%esp)
	pushl 44(%esp) 
	pushl 44(%esp) 
	pushl 64(%esp)
	pushl 76(%esp)
	pushl 76(%esp)
	pushl $errmsg
	call vga_printf
	jmp halt

.section .data
errmsg:
	.ascii "%s (code=%x)\n"
	.ascii "EAX=%08x   ESP=%08x\n"
	.ascii "EBX=%08x   EBP=%08x\n"
	.ascii "ECX=%08x   ESI=%08x\n"
	.ascii "EDX=%08x   EDI=%08x\n"
	.asciz "EIP=%08x   CS=%04x\n"

msg_UNK: .asciz "Unknown Exception"
msg_DE: .asciz "Divide-by-zero Error"
msg_DB: .asciz "Debug"
msg_NMI: .asciz "Non-maskable Interrupt"
msg_BP: .asciz "Breakpoint"
msg_OF: .asciz "Overflow"
msg_BR: .asciz "Bounded Range Exceeded"
msg_UD: .asciz "Invalid Opcode"
msg_NM: .asciz "Device Not Available"
msg_DF: .asciz "Double Fault"
msg_TS: .asciz "Invalid TSS"
msg_NP: .asciz "Segment Not Present"
msg_SS: .asciz "Stack-Segment Fault"
msg_GP: .asciz "General Protection Fault"
msg_PF: .asciz "Page Fault"
msg_MF: .asciz "Floating-Point Exception"
msg_AC: .asciz "Alignment Check"
msg_MC: .asciz "Machine Check"
msg_XM: .asciz "SIMD Floating-Point Exception"
msg_VE: .asciz "Virtualization Exception"
msg_SX: .asciz "Security Exception"
