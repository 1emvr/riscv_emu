#ifndef VMCRYPT_H
#define VMCRYPT_H
#include "vmmain.h"

namespace rvm64::crypt {
    __inline __function uintptr_t decrypt_ptr(uintptr_t ptr);
    constexpr uintptr_t encrypt_ptr(uintptr_t ptr);
};
#endif //VMCRYPT_H
