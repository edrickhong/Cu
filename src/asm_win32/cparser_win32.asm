

PUBLIC InternalFillArgsAndCall


_DATA SEGMENT
_DATA ENDS

	_TEXT	SEGMENT
	
	;; void* call,u64* values,u64* iret,f64* fret
	;; RCX, RDX, R8, R9
	;; XMM0, XMM1, XMM2, XMM3
	
	;; RAX, |RCX, RDX, R8, R9|, R10, R11 are considered volatile
	
	InternalFillArgsAndCall PROC

	mov qword ptr [rsp + 20h] , r9	;fret
	mov qword ptr [rsp + 18h] , r8	;iret
	mov qword ptr [rsp + 10h] , rdx ;values
	mov qword ptr [rsp + 8h] , rcx ;call
	
	sub rsp,20h

	;; load values
	mov rax,rdx

	mov qword ptr rcx,[rax]
	mov qword ptr r8,[rax + 10h]

	mov qword ptr r10,[rax + 8h]
	mov qword ptr r11,[rax + 18h]

	movq xmm1,r10
	movq xmm3,r11
	
	;; mov qword ptr rcx,[rax]
	;; mov qword ptr rdx,[rax + 8h]
	;; mov qword ptr r8,[rax + 10h]
	;; mov qword ptr r9,[rax + 18h]

	;; mov qword ptr r10,[rax + 20h]
	;; mov qword ptr r11,[rax + 28h]

	;; TODO:this can't work. when faced with mixed args msvc does
	;; rcx xmm1 r8 xmm3

	
	;; movq xmm0,r10
	;; movq xmm1,r11

	;; mov qword ptr r10,[rax + 30h]
	;; mov qword ptr r11,[rax + 38h]

	;; movq xmm2,r10
	;; movq xmm3,r11

	mov qword ptr r10,[rsp + 28h]
	call r10

	
	add rsp,20h

	;; write the return values
	mov qword ptr r8,[rsp + 18h]
	mov qword ptr r9,[rsp + 20h]

	mov qword ptr [r8],rax
	movq r8,xmm0
	mov qword ptr [r9],r8
	
	ret

InternalFillArgsAndCall ENDP

_TEXT	ENDS

END
