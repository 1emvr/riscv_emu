#include <errhandlingapi.h>

#include "vmmain.hpp"
#include "vmcode.hpp"
#include "vmcommon.hpp"
#include "vmveh.hpp"
#include "rvni.hpp"
#include "mock.hpp"

namespace rvm64::entry {
	_native bool vm_init() {
		vmcs->dkey = key; 
		vmcs->handler = (uintptr_t)handler;
		vmcs->trap_handler = (uintptr_t)&&vm_exit;

		AddVectoredExceptionHandler(1, vm_exception_handler);

		rvm64::memory::context_init();
		rvm64::rvni::resolve_ucrt_imports(); 

		if (!rvm64::mock::read_program_from_packet()) {
			return false;
		}
		return true;
	}

	_native void vm_end() {
		rvm64::memory::memory_end();
		RemoveVectoredExceptionHandler(vm_exception_handler);
	}

	_vmcall void vm_entry() {
		while (!vmcs->halt) {
			vmcs->step = true;
			rvm64::decoder::vm_decode(*(int32_t*)vmcs->pc);

			if (vmcs->csr.m_cause == environment_call_native) {
				rvni::vm_trap_exit();  
				continue;
			}
			if (vmcs->step) {
				vmcs->pc += 4; 
			}
		}
	}
};

namespace rvm64 {
	_native int64_t vm_main() {
		vmcs_t vm_instance = { };
		vmcs = &vm_instance;

		rvm64::entry::vm_init();
		rvm64::entry::vm_entry();

	vm_exit:
		rvm64::entry::vm_end();
		return (int64_t)vmcs->csr.m_cause;
	}
};

_native int main() {
    rvm64::vm_main();
}

