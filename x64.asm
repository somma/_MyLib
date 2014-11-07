;;=============================================================================
; x64 assembly with visual studio
;;=============================================================================

.code

; void _break();
_break PROC
    int 3
    ret
_break ENDP



; ULONG64 sum64(ULONG64 a, ULONG64 b, ULONG64 c, ULONG64 d)
;                     rcx,       rdx,        r8,        r9
sum64 proc
	xor rax, rax
	add rax, rcx
	add rax, rdx
	add rax, r8
	add rax, r9	
	ret
sum64 endp


; trampolines in x64 env
; void trampoline()
trampoline proc	
	mov rax, 0abcdabcdffff0000h
	jmp rax
trampoline endp


; direct jump 
direct_jump proc
	push rbx
	xor rbx, rbx

	jmp _stop			; direct near jump
	mov rbx, 0			; never called
	_stop:
		mov rbx, 01000h
	
	mov rax, rbx
	pop rbx
	ret
direct_jump endp

; indirect jump 
indirect_jump proc
	push rbx
	xor rbx, rbx

	mov rbx, offset _stop
	jmp rbx
	mov rbx, 0				; never called
	_stop:
		mov rbx, 01000h
	mov rax, rbx
	pop rbx
	ret
indirect_jump endp

; push, move rsp+4, ret
push_mov_ret proc
	push rdx
	push rbx
		
	mov rdx, 000000000ffffffffh
	mov rbx, sum64
	and rbx, rdx
	push rbx
	
	mov rdx, 0ffffffff00000000h
	mov rbx, sum64
	and rbx, rdx
	shr rbx, 20h
	
	mov [rsp+4], ebx	; adjust qword value

	pop rbx				; rbx == sum64
	pop rbx
	pop rdx
	ret
push_mov_ret endp


; void push_mov_ret2();
; jump 0xaabbccdd11223344
;
push_mov_ret2 proc	
	push 011223344h
	mov dword ptr [rsp+4], 0aabbccddh
	ret
push_mov_ret2 endp


end
