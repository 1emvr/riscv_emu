#include <errhandlingapi.h>

#include "vmmain.hpp"
#include "vmcode.hpp"
#include "vmcommon.hpp"
#include "vmveh.hpp"
#include "rvni.hpp"
#include "mock.hpp"

namespace rvm64::entry {
	_native void vm_init() {
		rvm64::context::save_host_context();

		vmcs->dkey = key;
		vmcs->handler = (uintptr_t)handler;
		vmcs->trap_handler = (uintptr_t)&&vm_return;

		AddVectoredExceptionHandler(1, vm_exception_handler);

		rvm64::memory::context_init();
		rvm64::rvni::resolve_ucrt_imports(); 
		rvm64::mock::read_program_from_packet();
	}

	_native void vm_exit() {
		rvm64::memory::memory_end();
		RemoveVectoredExceptionHandler(vm_exception_handler);
		rvm64::context::restore_host_context();
	}

	_vmcall void vm_entry() {
		while (!vmcs->halt) {
			rvm64::decoder::vm_decode(*(int32_t*)vmcs->pc);
			vmcs->pc += 4;
		}
	}
};

namespace rvm64 {
	_native int64_t vm_main() {
		vmcs_t vm_instance = { };
		vmcs = &vm_instance;

		rvm64::entry::vm_init();
		rvm64::entry::vm_entry();

	vm_return:
		rvm64::entry::vm_exit();
		return (int64_t)vmcs->csr.m_cause;
	}
};

_native int main() {
    rvm64::vm_main();
}

