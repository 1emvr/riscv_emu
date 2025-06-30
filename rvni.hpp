#ifndef RVNI_H
#define RVNI_H
#include <unordered_map>
#include <windows.h>

#include "vmmain.hpp"
#include "vmcommon.hpp"
#include "vmrwx.hpp"

namespace rvm64::rvni {
	// NOTE: this requires C++ stdlib to be statically linked. consider replacing with simple_map::unordered_map.
	_data std::unordered_map<void*, ucrt_wrapper> ucrt_native_table;

	struct function {
		const char* name;
		ucrt_wrapper::typenum type;
	};

	_rdata ucrt_alias alias_table[] = {
		{ "open",  "_open"  }, { "read",  "_read"  }, { "write", "_write" },
		{ "close", "_close" }, { "exit",  "_exit"  },
	};

	_rdata function unresolved[] = {
		{"_open", ucrt_wrapper::PLT_OPEN}, {"_read", ucrt_wrapper::PLT_READ}, {"_write", ucrt_wrapper::PLT_WRITE}, {"_close", ucrt_wrapper::PLT_CLOSE},
		{"_lseek", ucrt_wrapper::PLT_LSEEK}, {"_stat64", ucrt_wrapper::PLT_STAT64}, {"malloc", ucrt_wrapper::PLT_MALLOC}, {"free", ucrt_wrapper::PLT_FREE},
		{"memcpy", ucrt_wrapper::PLT_MEMCPY}, {"memset", ucrt_wrapper::PLT_MEMSET}, {"strlen", ucrt_wrapper::PLT_STRLEN}, {"strcpy", ucrt_wrapper::PLT_STRCPY},
	};

	_native void* windows_thunk_resolver(const char* sym_name) {
		static HMODULE ucrt = LoadLibraryA("ucrtbase.dll");
		if (!ucrt) {
			CSR_SET_TRAP(nullptr, image_bad_symbol, 0, 0, 1);
		}

		for (auto &i : alias_table) {
			if (strcmp(sym_name, i.original) == 0) {
				sym_name = i.alias;
				break;
			}
		}

		void* proc = (void*)GetProcAddress(ucrt, sym_name);
		if (!proc) {
			CSR_SET_TRAP(nullptr, image_bad_symbol, 0, 0, 1);
		}

		return proc;
	}

	_native void resolve_ucrt_imports() {
		HMODULE ucrt = LoadLibraryA("ucrtbase.dll");
		if (!ucrt) {
			CSR_SET_TRAP(nullptr, image_bad_symbol, 0, 0, 1);
		}

		for (auto& f : unresolved) {
			void* native = (void*)GetProcAddress(ucrt, f.name);
			if (!native) {
				CSR_SET_TRAP(nullptr, image_bad_symbol, 0, 0, 1);
			}

			ucrt_wrapper wrap = { };
			wrap.address = native;
			wrap.type = f.type;

			switch (wrap.type) {
				case ucrt_wrapper::PLT_OPEN:    wrap.open = (decltype(wrap.open))native; break;
				case ucrt_wrapper::PLT_READ:    wrap.read = (decltype(wrap.read))native; break;
				case ucrt_wrapper::PLT_WRITE:   wrap.write = (decltype(wrap.write))native; break;
				case ucrt_wrapper::PLT_CLOSE:   wrap.close = (decltype(wrap.close))native; break;
				case ucrt_wrapper::PLT_LSEEK:   wrap.lseek = (decltype(wrap.lseek))native; break;
				case ucrt_wrapper::PLT_STAT64:  wrap.stat64 = (decltype(wrap.stat64))native; break;
				case ucrt_wrapper::PLT_MALLOC:  wrap.malloc = (decltype(wrap.malloc))native; break;
				case ucrt_wrapper::PLT_FREE:    wrap.free = (decltype(wrap.free))native; break;
				case ucrt_wrapper::PLT_MEMCPY:  wrap.memcpy = (decltype(wrap.memcpy))native; break;
				case ucrt_wrapper::PLT_MEMSET:  wrap.memset = (decltype(wrap.memset))native; break;
				case ucrt_wrapper::PLT_STRLEN:  wrap.strlen = (decltype(wrap.strlen))native; break;
				case ucrt_wrapper::PLT_STRCPY:  wrap.strcpy = (decltype(wrap.strcpy))native; break;
				default: {
					CSR_SET_TRAP(nullptr, image_bad_symbol, 0, 0, 1);
				}
			}

			ucrt_native_table[native] = wrap;
		}
	}

	_vmcall void vm_native_call() {
		auto it = ucrt_native_table.find((void*)vmcs->pc);
		if (it == ucrt_native_table.end()) {
			CSR_SET_TRAP(vmcs->pc, illegal_instruction, 0, vmcs->pc, 1);
		}

		ucrt_wrapper &plt = it->second;
		switch (plt.type) {
			case ucrt_wrapper::PLT_OPEN: {
				char *pathname;
				int flags = 0, mode = 0;

				reg_read(char*, pathname, regenum::a0);
				reg_read(int, flags, regenum::a1);
				reg_read(int, mode, regenum::a2);

				SAVE_VM_CONTEXT(int result = plt.open(pathname, flags, mode));
				reg_write(int, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_READ: {
				int fd = 0;
				void *buf;
				unsigned int count = 0;

				reg_read(int, fd, regenum::a0);
				reg_read(void*, buf, regenum::a1);
				reg_read(unsigned int, count, regenum::a2);

				SAVE_VM_CONTEXT(int result = plt.read(fd, buf, count));
				reg_write(int, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_WRITE: {
				int fd = 0;
				void *buf;
				unsigned int count = 0;

				reg_read(int, fd, regenum::a0);
				reg_read(void*, buf, regenum::a1);
				reg_read(unsigned int, count, regenum::a2);

				SAVE_VM_CONTEXT(int result = plt.write(fd, buf, count));
				reg_write(int, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_CLOSE: {
				int fd = 0;
				reg_read(int, fd, regenum::a0);

				SAVE_VM_CONTEXT(int result = plt.close(fd));
				reg_write(int, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_LSEEK: {
				int fd = 0;
				long offset = 0;
				int whence = 0;

				reg_read(int, fd, regenum::a0);
				reg_read(long, offset, regenum::a1);
				reg_read(int, whence, regenum::a2);

				SAVE_VM_CONTEXT(long result = plt.lseek(fd, offset, whence));
				reg_write(long, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_STAT64: {
				const char *pathname;
				void *statbuf;

				reg_read(const char*, pathname, regenum::a0);
				reg_read(void*, statbuf, regenum::a1);

				SAVE_VM_CONTEXT(int result = plt.stat64(pathname, statbuf));
				reg_write(int, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_MALLOC: {
				size_t size = 0;
				reg_read(size_t, size, regenum::a0);

				SAVE_VM_CONTEXT(void* result = plt.malloc(size));
				reg_write(uintptr_t, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_FREE: {
				void *ptr;
				reg_read(void*, ptr, regenum::a0);

				SAVE_VM_CONTEXT(plt.free(ptr));
				break;
			}
			case ucrt_wrapper::PLT_MEMCPY: {
				void *dest, *src;
				size_t n = 0;

				reg_read(void*, dest, regenum::a0);
				reg_read(void*, src, regenum::a1);
				reg_read(size_t, n, regenum::a2);

				SAVE_VM_CONTEXT(void* result = plt.memcpy(dest, src, n));
				reg_write(uintptr_t, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_MEMSET: {
				void *dest;
				int value = 0;
				size_t n = 0;

				reg_read(void*, dest, regenum::a0);
				reg_read(int, value, regenum::a1);
				reg_read(size_t, n, regenum::a2);

				SAVE_VM_CONTEXT(void* result = plt.memset(dest, value, n));
				reg_write(uint64_t, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_STRLEN: {
				char *s;
				reg_read(char*, s, regenum::a0);

				SAVE_VM_CONTEXT(size_t result = plt.strlen(s));
				reg_write(size_t, regenum::a0, result);
				break;
			}
			case ucrt_wrapper::PLT_STRCPY: {
				char *dest, *src;

				reg_read(char*, dest, regenum::a0);
				reg_read(char*, src, regenum::a1);

				SAVE_VM_CONTEXT(char* result = plt.strcpy(dest, src));
				reg_write(uintptr_t, regenum::a0, result);
				break;
			}
			default: {
				CSR_SET_TRAP(vmcs->pc, illegal_instruction, 0, plt.type, 1);
			}
		}

		uintptr_t ret = 0;
		reg_read(uintptr_t, ret, regenum::ra);
		vmcs->pc = ret;
	}
}
#endif // RVNI_H
