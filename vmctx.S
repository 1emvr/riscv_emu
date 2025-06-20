.intel_syntax noprefix
#include "vmctx.inc"

.text
.extern vmcs

.global save_host_context
save_host_context:
    mov     rax, [rip + vmcs]
    mov     rax, [rax]
    lea     rdi, [rax + OFFSET_host_context]

    mov     [rdi + OFFSET_RAX], rax
    mov     [rdi + OFFSET_RBX], rbx
    mov     [rdi + OFFSET_RCX], rcx
    mov     [rdi + OFFSET_RDX], rdx
    mov     [rdi + OFFSET_RSI], rsi
    mov     [rdi + OFFSET_RDI], rdi
    mov     [rdi + OFFSET_RBP], rbp

    mov     [rdi + OFFSET_R8],  r8
    mov     [rdi + OFFSET_R9],  r9
    mov     [rdi + OFFSET_R10], r10
    mov     [rdi + OFFSET_R11], r11
    mov     [rdi + OFFSET_R12], r12
    mov     [rdi + OFFSET_R13], r13
    mov     [rdi + OFFSET_R14], r14
    mov     [rdi + OFFSET_R15], r15

    pushfq
    pop     qword ptr [rdi + OFFSET_FLAGS]

    ret

.global restore_host_context
restore_host_context:
    mov     rax, [rip + vmcs]
    mov     rax, [rax]
    lea     rsi, [rax + OFFSET_host_context]

    mov     rax, [rsi + OFFSET_RAX]
    mov     rbx, [rsi + OFFSET_RBX]
    mov     rcx, [rsi + OFFSET_RCX]
    mov     rdx, [rsi + OFFSET_RDX]
    mov     rsi, [rsi + OFFSET_RSI]
    mov     rdi, [rsi + OFFSET_RDI]
    mov     rbp, [rsi + OFFSET_RBP]

    mov     r8,  [rsi + OFFSET_R8]
    mov     r9,  [rsi + OFFSET_R9]
    mov     r10, [rsi + OFFSET_R10]
    mov     r11, [rsi + OFFSET_R11]
    mov     r12, [rsi + OFFSET_R12]
    mov     r13, [rsi + OFFSET_R13]
    mov     r14, [rsi + OFFSET_R14]
    mov     r15, [rsi + OFFSET_R15]

    push    qword ptr [rsi + OFFSET_FLAGS]
    popfq

    ret

.global save_vm_context
save_vm_context:
    mov     rax, [rip + vmcs]
    mov     rax, [rax]
    lea     rdi, [rax + OFFSET_vm_context]

    mov     [rdi + OFFSET_RAX], rax
    mov     [rdi + OFFSET_RBX], rbx
    mov     [rdi + OFFSET_RCX], rcx
    mov     [rdi + OFFSET_RDX], rdx
    mov     [rdi + OFFSET_RSI], rsi
    mov     [rdi + OFFSET_RDI], rdi
    mov     [rdi + OFFSET_RBP], rbp

    mov     [rdi + OFFSET_R8],  r8
    mov     [rdi + OFFSET_R9],  r9
    mov     [rdi + OFFSET_R10], r10
    mov     [rdi + OFFSET_R11], r11
    mov     [rdi + OFFSET_R12], r12
    mov     [rdi + OFFSET_R13], r13
    mov     [rdi + OFFSET_R14], r14
    mov     [rdi + OFFSET_R15], r15

    pushfq
    pop     qword ptr [rdi + OFFSET_FLAGS]

    ret

.global restore_vm_context
restore_vm_context:
    mov     rax, [rip + vmcs]
    mov     rax, [rax]
    lea     rsi, [rax + OFFSET_vm_context]

    mov     rax, [rsi + OFFSET_RAX]
    mov     rbx, [rsi + OFFSET_RBX]
    mov     rcx, [rsi + OFFSET_RCX]
    mov     rdx, [rsi + OFFSET_RDX]
    mov     rsi, [rsi + OFFSET_RSI]
    mov     rdi, [rsi + OFFSET_RDI]
    mov     rbp, [rsi + OFFSET_RBP]

    mov     r8,  [rsi + OFFSET_R8]
    mov     r9,  [rsi + OFFSET_R9]
    mov     r10, [rsi + OFFSET_R10]
    mov     r11, [rsi + OFFSET_R11]
    mov     r12, [rsi + OFFSET_R12]
    mov     r13, [rsi + OFFSET_R13]
    mov     r14, [rsi + OFFSET_R14]
    mov     r15, [rsi + OFFSET_R15]

    push    qword ptr [rsi + OFFSET_FLAGS]
    popfq

    ret
