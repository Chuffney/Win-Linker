global initBufferObject ;void (void*)
global addString        ;uint32_t (stringBuffer*, char*)
global addChunk         ;uint32_t (binaryBuffer, void* chunk, uint32_t length)

extern malloc
extern realloc
extern memcpy

%define startingSize 512

struc Header
    .bufferPtr:     resq 1
    .allocMemory:   resd 1
    .currentOffset: resd 1
endstruc

section .text

initBufferObject:
    push rbx
    sub rsp, 0x20

    mov rbx, rcx
    mov ecx, startingSize
    mov [rbx + Header.allocMemory], rcx ;allocMemory:currentOffset - both in rcx
    call malloc
    mov [rbx + Header.bufferPtr], rax

    add rsp, 0x20
    pop rbx
    ret

addString:
    mov r8, [rcx + Header.allocMemory]
    mov r9d, r8d
    shr r8, 0x20
    sub r9d, r8d    ;remaining bytes till end of buffer
    mov r10, [rcx + Header.bufferPtr]
    add r8, r10    ;address of the first free byte in buffer

    AS_loop:
        test r9d, r9d
        jz AS_realloc
    AS_realloc_ret:
        mov al, [rdx]
        mov [r8], al
        inc rdx
        inc r8
        dec r9d
        test al, al
        jnz AS_loop
    sub r8, r10
    mov eax, r8d
    xchg eax, [rcx + Header.currentOffset]
    ret

AS_realloc:
    sub r8, r10     ;relative offset
    push r8
    push rcx
    push rdx
    sub rsp, 0x20

    lea rdx, [r8 * 2]
    mov [rcx + Header.allocMemory], edx
    mov rcx, r10
    call realloc
    mov r10, rax

    add rsp, 0x20
    pop rdx
    pop rcx
    pop r9  ;remaining bytes - buffer doubles in size so this value is equal to current offset
    lea r8, [r9 + rax]
    mov [rcx + Header.bufferPtr], rax
    jmp AS_realloc_ret


addChunk:
    mov r8d, r8d
    mov rax, [rcx + Header.allocMemory]
    mov r9d, eax
    shr rax, 0x20
    sub r9d, eax    ;remaining allocated memory in the buffer
    add eax, r8d    ;offset after the new chunk is added
    mov [rcx + Header.currentOffset], eax
    sub eax, r8d    ;back to current offset
    cmp r8d, r9d    ;cmp neededMem, availableMem
    ja AC_realloc
    mov rcx, [rcx + Header.bufferPtr]
AC_realloc_ret:
    add rcx, rax

    push rax
    sub rsp, 0x20
    call memcpy

    add rsp, 0x20
    pop rax
    ret

AC_realloc:
    push rax
    push rcx
    push rdx
    push r8
    sub rsp, 0x20

    AC_doubleMem_loop:
        add r9d, eax   ;total alloc'd memory
        shl r9d, 1
        sub r9d, eax   ;free mem after doubling
        cmp r8d, r9d   ;cmp neededMem, availableMem
        ja AC_doubleMem_loop
    lea rdx, [r9 + rax]
    mov [rcx + Header.allocMemory], edx
    mov rcx, [rcx + Header.bufferPtr]
    call realloc

    add rsp, 0x20
    pop r8
    pop rdx
    pop rcx
    mov [rcx + Header.bufferPtr], rax
    mov rcx, rax
    pop rax
    jmp AC_realloc_ret
