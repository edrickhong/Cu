

PUBLIC InternalFillArgsAndCall


_DATA SEGMENT
_DATA ENDS

	_TEXT	SEGMENT
	
	;; void* call,u64* values,u64* iret,f64* fret
	;; RCX, RDX, R8, R9
	;; XMM0, XMM1, XMM2, XMM3
	
	;; RAX, |RCX, RDX, R8, R9|, R10, R11 are considered volatile
	
	InternalFillArgsAndCall PROC

	push rbp
	mov rbp,rsp

	mov qword ptr [rsp + 28h] , r9	;fret
	mov qword ptr [rsp + 20h] , r8	;iret
	mov qword ptr [rsp + 18h] , rdx ;values
	mov qword ptr [rsp + 10h] , rcx ;call
	
	sub rsp,20h

	;; load values
	mov rax,rdx
	
	mov qword ptr rcx,[rax]
	mov qword ptr rdx,[rax + 8h]
	mov qword ptr r8,[rax + 10h]
	mov qword ptr r9,[rax + 18h]

	movss xmm0,[rax + 20h]
	movss xmm1,[rax + 28h]

	movss xmm2,[rax + 30h]
	movss xmm3,[rax + 38h]

	mov qword ptr r10,[rsp + 30h]
	call r10

	
	add rsp,20h

	;; write the return values
	mov qword ptr r8,[rsp + 20h]
	mov qword ptr r9,[rsp + 28h]

	mov qword ptr [r8],rax
	movq r8,xmm0
	mov qword ptr [r9],r8

	pop rbp
	
	ret

InternalFillArgsAndCall ENDP

_TEXT	ENDS

END
