#include "vmmain.hpp"
#include "vmcode.hpp"
#include "vmcommon.hpp"
#include "rvni.hpp"
#include "mock.hpp"

namespace rvm64::entry {
	__native void vm_init() {
		rvm64::memory::context_init();
		rvm64::rvni::resolve_ucrt_imports(); 

		while (!vmcs->halt) {
			if (!rvm64::mock::read_program_from_packet()) { 
				continue;
			}
		}
	}

	__native void vm_end() {
		rvm64::memory::memory_end();
	}

	__vmcall void vm_entry() {
		while (!vmcs->halt) {
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
	__native int64_t vm_main() {
		vmcs_t vm_instance = { };
		vmcs = &vm_instance;

		rvm64::entry::vm_init();
		rvm64::entry::vm_entry();
		rvm64::entry::vm_end();

		return (int64_t)vmcs->reason;
	}
};

__native int main() {
    rvm64::vm_main();
}

