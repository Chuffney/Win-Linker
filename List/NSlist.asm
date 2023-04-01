;x86_64, NASM-style assembly
;uses the C Standard Library
;MS calling convention

%include "common.inc"

extern malloc
extern realloc
extern free
extern memcpy
extern memcmp

global NS_newList
global NS_initList
global freeList
global NS_add
global NS_addRef
global NS_removeElement
global NS_removeIndex
global NS_getIndex

section .text
NS_newList:
    push rbx
    sub rsp, 0x20
    mov bl, cl ;save element size for later
    mov ecx, List_size
    call malloc
    mov dl, bl
    mov rcx, rax
    jmp IL_skipRbxPush

NS_initList:
    push rbx
    sub rsp, 0x20
IL_skipRbxPush:
    mov dword [rcx + List.currentLength], 0
    mov dword [rcx + List.allocatedMem], defaultSize
    mov [rcx + List.elementSize], dl
    mov rbx, rcx
    mov ecx, defaultSize
    call malloc
    mov [rbx + List.arrayPtr], rax
    add rsp, 0x20
    pop rbx
    ret

NS_add:
    push rdx
    push rcx
    call assureSpace
    pop rcx
    pop rdx
    inc dword [rcx + List.currentLength]
    mov [rax], rdx
    ret

NS_addRef:
    push rdx
    push rcx
    call assureSpace
    pop rcx
    pop rdx
    push rax
    inc dword [rcx + List.currentLength]
    mov r8b, [rcx + List.elementSize]
    mov rcx, rax
    movzx r8d, r8b
    sub rsp, 0x20
    call memcpy
    add rsp, 0x20
    pop rax
    ret

assureSpace:   ;void(List*)     returns first free adress
    mov eax, [rcx + List.currentLength]
    mov r8b, [rcx + List.elementSize]
    movzx r8d, r8b
    mul r8d ;currently used space in bytes in eax
    mov edx, [rcx + List.allocatedMem]
    sub edx, eax    ;free allocated bytes in edx
    cmp edx, 8      ;{
    jb AS_realloc   ;if (allocatedMem - (currentLength * elementSize) < 8 || allocatedMem - (currentLength * elementSize) < elementSize) then realloc
    cmp edx, r8d    ;}
    jb AS_realloc
    add rax, [rcx + List.arrayPtr]
    ret
AS_realloc:
    add edx, eax    ;reverting edx to holding allocatedMem
    shl edx, 1
    mov [rcx + List.allocatedMem], edx
    push rcx
    mov rcx, [rcx + List.arrayPtr]
    sub rsp, 0x20
    call realloc
    add rsp, 0x20
    pop rcx
    mov [rcx + List.arrayPtr], rax
    jmp assureSpace

NS_getIndex:
    push rbx    ;elementSize
    push rbp    ;elementPtr
    push rdi    ;arrayPtr
    push rsi    ;iterator:currentLength
    sub rsp, 0x20
    movzx ebx, byte [rcx + List.elementSize]
    mov rbp, rdx
    mov rdi, [rcx + List.arrayPtr]
    mov esi, [rcx + List.currentLength]
    jmp GI_loopControl
GI_loop:
        mov rcx, rdi
        mov rdx, rbp
        mov r8d, ebx
        call memcmp
        test eax, eax   ;memcmp returns zero if element in array is equal to the searched element
        mov eax, esi
        jz GI_retSuccess
GI_loopControl:
        add rdi, rbx
        rol rsi, 0x20
        inc esi
        cmp esi, eax    ;cmp iterator, currentLength
        rol rsi, 0x20
        jne GI_loop
;GI_retNotFound:
    mov eax, 0xFFFFFFFF
GI_retSuccess:
    add rsp, 0x20
    pop rsi
    pop rdi
    pop rbp
    pop rbx
    ret

NS_removeIndex:
    mov eax, edx
    movzx r8d, byte [rcx + List.elementSize]
    mul r8d
    mov r9d, eax
    mov eax, [rcx + List.currentLength]
    dec eax
    mov [rcx + List.currentLength], eax
    mul r8d

    ;destination index in r9d, source in eax
    mov edx, eax
    mov rcx, [rcx + List.arrayPtr]
    add rdx, rcx
    add rcx, r9
    call memcpy
    ret

NS_removeElement:
    push rcx
    call NS_getIndex
    pop rcx
    mov edx, eax
    inc eax ;check if return of getIndex was -1
    mov eax, edx
    jz RE_ret
    call NS_removeIndex
RE_ret:
    ret
