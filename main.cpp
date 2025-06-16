#include "vmmain.hpp"
#include "vmcode.hpp"
#include "vmcommon.hpp"
#include "rvni.hpp"
#include "mock.hpp"

namespace rvm64::entry {
	_native void vm_init() {
		rvm64::memory::context_init();
		rvm64::rvni::resolve_ucrt_imports(); 

		while (!vmcs->halt) {
			if (!rvm64::mock::read_program_from_packet()) { 
				continue;
			}
		}
	}

	_native void vm_end() {
		rvm64::memory::memory_end();
	}

	_vmcall void vm_entry() {
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
	_native int64_t vm_main() {
		vmcs_t vm_instance = { };
		vmcs = &vm_instance;

		vmcs->dkey = key; 
		vmcs->handler = (uintptr_t)handler;

		vmcs->load_rsv_valid = false;
		vmcs->load_rsv_addr = 0LL;

		vmcs->csr.m_cause = 0;                                
		vmcs->csr.m_epc = 0;                                           
		vmcs->csr.m_tval = 0;                                            
		vmcs->halt = 0;

		rvm64::entry::vm_init();
		rvm64::entry::vm_entry();
		rvm64::entry::vm_end();

		return (int64_t)vmcs->reason;
	}
};

_native int main() {
    rvm64::vm_main();
}

