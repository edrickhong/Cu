

PUBLIC InternalFillArgsAndCall


_DATA SEGMENT
_DATA ENDS

	_TEXT	SEGMENT
	
	;; void* call,u64* values,u64* iret,f64* fret
	;; RCX, RDX, R8, R9
	;; XMM0, XMM1, XMM2, XMM3
	;; rax rsi rdi r10 r11 r12 r13 r14 r15

	;; rbx rdi rdsi :: r12 r13 r14
	
	InternalFillArgsAndCall PROC

	push rbx
	push rdi
	push rsi
	push r12

	push r13
	push r14
	push rax
	push rcx
	
	mov rax,rcx 		;call
	mov rsi,rdx		;values
	mov rdi,r8		;iret
	mov rbx,r9		;fret

	;; fill registers

	mov rcx,[rsi]
	mov rdx,[rsi + 8]
	mov r8,[rsi + 16]
	mov r9,[rsi + 24]

	mov r11,[rsi + 8]
	mov r12,[rsi + 8]
	mov r13,[rsi + 16]
	mov r14,[rsi + 24]

	movd xmm0,r11
	movd xmm1,r12
	movd xmm2,r13
	movd xmm3,r14
	
	call rax
	

	mov [rdi],rax

	movd r11,xmm0
	mov [rbx],r11

	pop rcx
	pop rax
	pop r14
	pop r13


	pop r12
	pop rsi
	pop rdi
	pop rbx
	
	ret

InternalFillArgsAndCall ENDP

_TEXT	ENDS

END
