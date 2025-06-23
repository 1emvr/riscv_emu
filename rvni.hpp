#ifndef RVNI_H
#define RVNI_H
#include <unordered_map>
#include <windows.h>

#include "vmmain.hpp"
#include "vmcommon.hpp"
#include "vmrwx.hpp"

namespace rvm64::rvni {
	_data ucrt_alias alias_table[] = {
		{ "open",  "_open"  }, { "read",  "_read"  }, { "write", "_write" },
		{ "close", "_close" }, { "exit",  "_exit"  },
	};

	struct function {
		const char* name;
		native_wrapper::typecaster type;
	};

	_data std::unordered_map<void*, native_wrapper> ucrt_native_table;

	_native void* windows_thunk_resolver(const char* sym_name) {
		static HMODULE ucrt = LoadLibraryA("ucrtbase.dll");
		if (!ucrt) {
			CSR_SET_TRAP(nullptr, image_bad_symbol, 0, 0, 1);
		}

		for (size_t i = 0; i < sizeof(alias_table) / sizeof(alias_table[0]); ++i) {
			if (strcmp(sym_name, alias_table[i].original) == 0) {
				sym_name = alias_table[i].alias;
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

		function funcs[] = {
			{"_open", native_wrapper::PLT_OPEN}, {"_read", native_wrapper::PLT_READ}, {"_write", native_wrapper::PLT_WRITE}, {"_close", native_wrapper::PLT_CLOSE},
			{"_lseek", native_wrapper::PLT_LSEEK}, {"_stat64", native_wrapper::PLT_STAT64}, {"malloc", native_wrapper::PLT_MALLOC}, {"free", native_wrapper::PLT_FREE},
			{"memcpy", native_wrapper::PLT_MEMCPY}, {"memset", native_wrapper::PLT_MEMSET}, {"strlen", native_wrapper::PLT_STRLEN}, {"strcpy", native_wrapper::PLT_STRCPY},
			{"printf", native_wrapper::PLT_PRINTF},
		};

		for (auto& f : funcs) {
			void* native = (void*)GetProcAddress(ucrt, f.name);
			if (!native) {
				CSR_SET_TRAP(nullptr, image_bad_symbol, 0, 0, 1);
			}

			native_wrapper wrap = { };
			wrap.address = native;
			wrap.type = f.type;

			switch (wrap.type) {
				case native_wrapper::PLT_OPEN:    wrap.open = (decltype(wrap.open))native; break;
				case native_wrapper::PLT_READ:    wrap.read = (decltype(wrap.read))native; break;
				case native_wrapper::PLT_WRITE:   wrap.write = (decltype(wrap.write))native; break;
				case native_wrapper::PLT_CLOSE:   wrap.close = (decltype(wrap.close))native; break;
				case native_wrapper::PLT_LSEEK:   wrap.lseek = (decltype(wrap.lseek))native; break;
				case native_wrapper::PLT_STAT64:  wrap.stat64 = (decltype(wrap.stat64))native; break;
				case native_wrapper::PLT_MALLOC:  wrap.malloc = (decltype(wrap.malloc))native; break;
				case native_wrapper::PLT_FREE:    wrap.free = (decltype(wrap.free))native; break;
				case native_wrapper::PLT_MEMCPY:  wrap.memcpy = (decltype(wrap.memcpy))native; break;
				case native_wrapper::PLT_MEMSET:  wrap.memset = (decltype(wrap.memset))native; break;
				case native_wrapper::PLT_STRLEN:  wrap.strlen = (decltype(wrap.strlen))native; break;
				case native_wrapper::PLT_STRCPY:  wrap.strcpy = (decltype(wrap.strcpy))native; break;
				case native_wrapper::PLT_PRINTF:  wrap.printf = (decltype(wrap.printf))native; break;
				default: {
					CSR_SET_TRAP(nullptr, image_bad_symbol, 0, 0, 1);
					return;
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

		native_wrapper &plt = it->second;
		switch (plt.type) {
			case native_wrapper::PLT_OPEN: {
				char *pathname;
				int flags = 0, mode = 0;

				reg_read(char*, pathname, regenum::a0);
				reg_read(int, flags, regenum::a1);
				reg_read(int, mode, regenum::a2);

				SAVE_VM_CONTEXT(int result = plt.open(pathname, flags, mode));
				reg_write(int, regenum::a0, result);
				break;
			}
			case native_wrapper::PLT_READ: {
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
			case native_wrapper::PLT_WRITE: {
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
			case native_wrapper::PLT_CLOSE: {
				int fd = 0;
				reg_read(int, fd, regenum::a0);

				SAVE_VM_CONTEXT(int result = plt.close(fd));
				reg_write(int, regenum::a0, result);
				break;
			}
			case native_wrapper::PLT_LSEEK: {
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
			case native_wrapper::PLT_STAT64: {
				const char *pathname;
				void *statbuf;

				reg_read(const char*, pathname, regenum::a0);
				reg_read(void*, statbuf, regenum::a1);

				SAVE_VM_CONTEXT(int result = plt.stat64(pathname, statbuf));
				reg_write(int, regenum::a0, result);
				break;
			}
			case native_wrapper::PLT_MALLOC: {
				size_t size = 0;
				reg_read(size_t, size, regenum::a0);

				SAVE_VM_CONTEXT(void* result = plt.malloc(size));
				reg_write(uintptr_t, regenum::a0, result);
				break;
			}
			case native_wrapper::PLT_FREE: {
				void *ptr;
				reg_read(void*, ptr, regenum::a0);

				SAVE_VM_CONTEXT(plt.free(ptr));
				break;
			}
			case native_wrapper::PLT_MEMCPY: {
				void *dest, *src;
				size_t n = 0;

				reg_read(void*, dest, regenum::a0);
				reg_read(void*, src, regenum::a1);
				reg_read(size_t, n, regenum::a2);

				SAVE_VM_CONTEXT(void* result = plt.memcpy(dest, src, n));
				reg_write(uintptr_t, regenum::a0, result);
				break;
			}
			case native_wrapper::PLT_MEMSET: {
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
			case native_wrapper::PLT_STRLEN: {
				char *s;
				reg_read(char*, s, regenum::a0);

				SAVE_VM_CONTEXT(size_t result = plt.strlen(s));
				reg_write(size_t, regenum::a0, result);
				break;
			}
			case native_wrapper::PLT_STRCPY: {
				char *dest, *src;

				reg_read(char*, dest, regenum::a0);
				reg_read(char*, src, regenum::a1);

				SAVE_VM_CONTEXT(char* result = plt.strcpy(dest, src));
				reg_write(uintptr_t, regenum::a0, result);
				break;
			}
			case native_wrapper::PLT_PRINTF: {
				// NOTE: this implementation is limited to 7 arguments max
				char *fmt;
				uint64_t args[7] = {};

				reg_read(char*, fmt, regenum::a0);
				for (int i = 1; i <= 7; ++i) {
					reg_read(uint64_t, args[i - 1], regenum::a0 + i);
				}

				SAVE_VM_CONTEXT(
					int result = plt.printf(fmt,
						args[0], args[1], args[2], args[3],
						args[4], args[5], args[6])
				);
				reg_write(int, regenum::a0, result);
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
