#include "vmmain.hpp"
#include "vmcode.hpp"
#include "vmcommon.hpp"

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

