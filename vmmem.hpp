#ifndef VMMEM_H
#define VMMEM_H

#include "vmctx.hpp"
#include "vmmain.hpp"
#include "vmops.hpp"

namespace rvm64::memory {
	__native bool vm_mem_init(size_t process_size) {
		vmcs->process.size = process_size;

		if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtAllocateVirtualMemory(NtCurrentProcess(), (LPVOID*) &vmcs->process.address, 
						0, &vmcs->process.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
			return false;
		}
	}

	__native void vm_end() {
		vmcs->reason = ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (LPVOID*)&vmcs->process.address, &vmcs->process.size, MEM_RELEASE);
	}
};
#endif // VMMEM_H
