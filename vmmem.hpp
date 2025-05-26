#ifndef VMMEM_H
#define VMMEM_H

#include "vmctx.hpp"
#include "vmmain.hpp"
#include "vmops.hpp"

/*
Main (Manager) Thread:
    - Listens for new RISC-V programs (VM images or bytecode).
    - For each new program, spins up a dedicated VM thread.
    - Keeps track of VM threads, their states, and communication channels.

Per-VM Thread:
    - Owns its own VM state (vmcs_t or equivalent).
    - Runs the RISC-V emulation loop for that program.
    - Handles VM instructions, memory, interrupts, etc.
    - Uses your atomic primitives (locks or reservations) to safely share any resources if needed.

Shared Resources:
    - If VMs share anything (memory regions, IO devices), protect those with fine-grained locks or atomic reservations.
    - Otherwise, keep VMs isolated for simplicity and better parallelism.

+----------------+
|     main()     |   <--- Main thread: starts the hypervisor manager thread
+----------------+
         |
         | creates
         v
+-------------------------+
| Hypervisor Manager Thread|  <--- Listens for new VM programs
+-------------------------+
         |
         | on new VM program:
         |  spin VM thread per program
         v
+---------------------+     +---------------------+    +---------------------+
| VM Thread (VM #1)    | ... | VM Thread (VM #2)    | ...| VM Thread (VM #N)    |
| Runs one RISC-V VM   |     | Runs another VM      |    | Runs another VM      |
+---------------------+     +---------------------+    +---------------------+

    */

namespace rvm64::memory {
    _function void vm_init() {
        rvm64::context::vm_context_init();

        vmcs->handler = (uintptr_t)rvm64::operation::__handler;
        vmcs->dkey = __key; // lol idk...

        vmcs->process.size = PROCESS_MAX_CAPACITY;

	if (vmcs->program.size > vmcs->process.size) {
		vmcs->halt = 1;
		vmcs->reason = vm_stack_overflow;
		return;
	}

        vmcs->load_rsv_valid = false;
        vmcs->load_rsv_addr = 0LL;

	if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtAllocateVirtualMemory(
					NtCurrentProcess(), (void**)&vmcs->process.address, 0, 
					&vmcs->process.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) 
	{
            vmcs->halt = 1;
        }
    }

    _function void vm_end() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtFreeVirtualMemory(
					NtCurrentProcess(), (LPVOID*)&vmcs->process.address, &vmcs->process.size, MEM_RELEASE))) 
	{
            vmcs->halt = 1;
        }
    }
};
#endif // VMMEM_H
