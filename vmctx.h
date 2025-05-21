#ifndef VMCTX_H
#define VMCTX_H

namespace rvm64::context {
    __function void vm_entry();
    __function void vm_context_init();
    __function void save_host_context();
    __function void restore_host_context();
    __function void save_vm_context();
    __function void restore_vm_context();
};
#endif //VMCTX_H
