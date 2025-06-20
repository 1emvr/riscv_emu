#ifndef VMCTX_H
#define VMCTX_H

#include <cstddef>
#include "vmmain.hpp"

#define OFFSET_RAX     offsetof(intel_t, rax)
#define OFFSET_RBX     offsetof(intel_t, rbx)
#define OFFSET_RCX     offsetof(intel_t, rcx)
#define OFFSET_RDX     offsetof(intel_t, rdx)
#define OFFSET_RSI     offsetof(intel_t, rsi)
#define OFFSET_RDI     offsetof(intel_t, rdi)
#define OFFSET_RBP     offsetof(intel_t, rbp)
#define OFFSET_R8      offsetof(intel_t, r8)
#define OFFSET_R9      offsetof(intel_t, r9)
#define OFFSET_R10     offsetof(intel_t, r10)
#define OFFSET_R11     offsetof(intel_t, r11)
#define OFFSET_R12     offsetof(intel_t, r12)
#define OFFSET_R13     offsetof(intel_t, r13)
#define OFFSET_R14     offsetof(intel_t, r14)
#define OFFSET_R15     offsetof(intel_t, r15)
#define OFFSET_FLAGS   offsetof(intel_t, rflags)

namespace rvm64::context {
    _naked _vmcall void save_host_context() {
        __asm {
            mov     rax, vmcs
            lea     rdi, [rax + offsetof(vmcs_t, host_context)]

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
        }
    }

    _naked _vmcall void restore_host_context() {
        __asm {
            mov     rax, vmcs
            lea     rsi, [rax + offsetof(vmcs_t, host_context)]

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
        }
    }

    _naked _vmcall void save_vm_context() {
        __asm {
            mov     rax, vmcs
            lea     rdi, [rax + offsetof(vmcs_t, vm_context)]

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
        }
    }

    _naked _vmcall void restore_vm_context() {
        __asm {
            mov     rax, vmcs
            lea     rsi, [rax + offsetof(vmcs_t, vm_context)]

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

            jmp     qword ptr [rsi + OFFSET_RIP]
        }
    }

}; // namespace rvm64::context

#endif // VMCTX_H
