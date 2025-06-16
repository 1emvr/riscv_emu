#ifndef VMCRYPT_HPP
#define VMCRYPT_HPP
#include "vmmain.hpp"

namespace rvm64::crypt {
	constexpr uintptr_t encrypt_ptr(uintptr_t ptr) {
		return ptr ^ __key;
	}

	_vmcall uintptr_t decrypt_ptr(uintptr_t ptr) {
		return ptr ^ vmcs->dkey;
	}
};

#define unwrap_opcall(hdl_idx) 									\
	do { 														\
		auto a = ((uintptr_t*)vmcs->handler)[hdl_idx];			\
		auto b = rvm64::crypt::decrypt_ptr((uintptr_t)a);		\
		void (*fn)() = (void (*)())(b);							\
		fn();													\
	} while(0)

#endif // VMCRYPT_HPP
