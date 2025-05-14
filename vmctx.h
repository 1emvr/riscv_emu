#ifndef VMCTX_H
#define VMCTX_H

namespace rvm64::context {
    __function void vm_entry(void);
    __function void vm_context_start();
    __function void save_host_context(void);
    __function void restore_host_context(void);
    __function void save_vm_context(void);
    __function void restore_vm_context(void);
};
#endif //VMCTX_H
