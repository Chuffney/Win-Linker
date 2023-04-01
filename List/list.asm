;x86_64, NASM-style assembly
;uses the C Standard Library
;MS calling convention

%include "common.inc"

extern malloc
extern realloc
extern free

global newList
global initList
global freeList
global add
global removeElement
global removeIndex
global getIndex

section .text
likelyRet:
    ret

newList:
    push rbx
    mov bl, cl    ;save elementSize for later
    sub rsp, 0x20
    mov ecx, List_size
    call malloc
    mov dword [rax + List.currentLength], 0
    mov dword [rax + List.allocatedMem], defaultSize
    cmp bl, 1
    je NL1
    cmp bl, 2
    je NL2
    cmp bl, 4
    je NL4
    cmp bl, 8
    je NL8
    xor eax, eax
    ret
    NL1:
        mov byte [rax + List.elementSize], 0
        jmp NL_collect
    NL2:
        mov byte [rax + List.elementSize], 1
        jmp NL_collect
    NL4:
        mov byte [rax + List.elementSize], 2
        jmp NL_collect
    NL8:
        mov byte [rax + List.elementSize], 3
NL_collect:
    mov rbx, rax
    mov ecx, defaultSize
    call malloc
    mov [rbx + List.arrayPtr], rax
    mov rax, rbx
    add rsp, 0x20
    pop rbx
    ret

initList:
    push rbx
    sub rsp, 0x20
IL_skipRbxPush:
    mov dword [rcx + List.currentLength], 0
    mov dword [rcx + List.allocatedMem], defaultSize
    cmp dl, 1
    je IL1
    cmp dl, 2
    je IL2
    cmp dl, 4
    je IL4
    cmp dl, 8
    je IL8
    xor eax, eax
    ret
IL1:
        mov byte [rcx + List.elementSize], 0
        jmp IL_collect
IL2:
        mov byte [rcx + List.elementSize], 1
        jmp IL_collect
IL4:
        mov byte [rcx + List.elementSize], 2
        jmp IL_collect
IL8:
        mov byte [rcx + List.elementSize], 3
IL_collect:
    mov rbx, rcx
    mov ecx, defaultSize
    call malloc
    mov [rbx + List.arrayPtr], rax

    mov rax, rbx
    add rsp, 0x20
    pop rbx
    ret

freeList:
    push rbx
    mov rbx, rcx    ;save struct pointer
    sub rsp, 0x20
    mov rcx, [rcx + List.arrayPtr]
    call free
    mov rcx, rbx    ;use the stored struct pointer
    call free
    add rsp, 0x20
    pop rbx
    ret

add:
    mov r8, rcx
    mov cl, [r8 + List.elementSize]
    mov eax, [r8 + List.currentLength]
    shl eax, cl
    cmp eax, [r8 + List.allocatedMem]
    je add_realloc
    jmp add_enoughSpace
add_realloc:
    push r8
    push rdx
    sub rsp, 0x20
    lea rdx, [rax * 2]  ;double the capacity
    mov rcx, [r8 + List.arrayPtr]
    mov [r8 + List.allocatedMem], edx
    call realloc
    add rsp, 0x20
    pop rdx
    pop r8
    mov [r8 + List.arrayPtr], rax  ;realloc'd memory
    mov cl, [r8 + List.elementSize]
add_enoughSpace:
    mov eax, [r8 + List.currentLength]
    inc dword [r8 + List.currentLength]
    mov r8, [r8 + List.arrayPtr]
    test cl, cl
    jz A1
    cmp cl, 2
    jb A2
    je A4
    ;jmp A8
A8:
    mov [r8 + rax * 8], rdx
    jmp add_SwitchCollect
A4:
    mov [r8 + rax * 4], edx
    jmp add_SwitchCollect
A2:
    mov [r8 + rax * 2], dx
    jmp add_SwitchCollect
A1:
    mov [r8 + rax], dl
add_SwitchCollect:
    ret

removeElement:
    push rbx
    mov rbx, rcx
    call getIndex
    mov edx, eax
    mov rcx, rbx
    pop rbx
    ;jmp removeIndex

removeIndex:
    mov al, [rcx + List.elementSize]
    mov r8d, [rcx + List.currentLength]
    dec r8d
    mov [rcx + List.currentLength], r8d
    mov rcx, [rcx + List.arrayPtr]
    test al, al
    jz RI1
    cmp al, 2
    jb RI2
    je RI4
    ;jmp RI8
RI8:
    mov rax, [rcx + r8 * 8]
    mov [rcx + rdx * 8], rax
    ret
RI4:
    mov eax, [rcx + r8 * 4]
    mov [rcx + rdx * 4], eax
    ret
RI2:
    mov ax, [rcx + r8 * 2]
    mov [rcx + rdx * 2], ax
    ret
RI1:
    mov al, [rcx + r8]
    mov [rcx + rdx], al
    ret

getIndex:
    mov rax, rcx
    mov ecx, [rax + List.currentLength]
    test ecx, ecx
    jz retFF32
    mov r8b, [rax + List.elementSize]
    mov rax, [rax + List.arrayPtr]
    test r8b, r8b
    jz GI1
    cmp r8b, 2
    jb GI2
    je GI4
    ;jmp GI8
GI8:
    dec ecx
    cmp rdx, [rax + rcx * 8]
    je GI_collect
    test ecx, ecx
    jnz GI8
    jmp retFF32
GI4:
    dec ecx
    cmp edx, [rax + rcx * 4]
    je GI_collect
    test ecx, ecx
    jnz GI4
    jmp retFF32
GI2:
    dec ecx
    cmp dx, [rax + rcx * 2]
    je GI_collect
    test ecx, ecx
    jnz GI2
    jmp retFF32
GI1:
    dec ecx
    cmp dl, [rax + rcx]
    je GI_collect
    test ecx, ecx
    jnz GI1
    jmp retFF32
GI_collect:
    mov eax, ecx
    ret

retFF32:
    mov eax, 0xFFFFFFFF
unlikelyRet:
    ret
