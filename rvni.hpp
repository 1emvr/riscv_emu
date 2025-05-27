#ifndef RVNI_H
#define RVNI_H
#include "vmmain.hpp"
#include "vmctx.hpp"

namespace rvm64::rvni {
	struct native_wrapper {
		void* raw_ptr;  
		enum typecaster {
			FUNC_OPEN, FUNC_READ, FUNC_WRITE, FUNC_CLOSE,
			FUNC_LSEEK, FUNC_STAT64, FUNC_MALLOC, FUNC_FREE,
			FUNC_MEMCPY, FUNC_MEMSET, FUNC_STRLEN, FUNC_STRCPY,
			FUNC_UNKNOWN
		} type;

		union {
			int (*open)(const char*, int, int);
			int (*read)(int, void*, unsigned int);
			int (*write)(int, const void*, unsigned int);
			int (*close)(int);
			long (*lseek)(int, long, int);
			int (*stat64)(const char*, void*);
			void* (*malloc)(size_t);
			void (*free)(void*);
			void* (*memcpy)(void*, const void*, size_t);
			void* (*memset)(void*, int, size_t);
			size_t (*strlen)(const char*);
			char* (*strcpy)(char*, const char*);
		};
	};

	_data std::unordered_map<void*, native_wrapper> ucrt_table; // NOTE: might cause compiler exception (unsure)

	_function void* windows_thunk_resolver(const char* sym_name) {
		static HMODULE ucrt = LoadLibraryA("ucrtbase.dll");
		if (!ucrt) {
			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			return nullptr;
		}

		// Alias table
		if (strcmp(sym_name, "open") == 0) sym_name = "_open";
		if (strcmp(sym_name, "read") == 0) sym_name = "_read";
		if (strcmp(sym_name, "write") == 0) sym_name = "_write";
		if (strcmp(sym_name, "close") == 0) sym_name = "_close";
		if (strcmp(sym_name, "exit") == 0) sym_name = "_exit";

		void* proc = (void*)GetProcAddress(ucrt, sym_name);
		if (!proc) {
			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			return nullptr;
		}

		return proc;
	}

	_function void resolve_ucrt_imports() {
		HMODULE ucrt = LoadLibraryA("ucrtbase.dll");

		if (!ucrt) {
			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			return;
		}

		struct {
			const char* name;
			native_wrapper::typecaster type;
		} funcs[] = {
			{"_open", native_wrapper::FUNC_OPEN}, {"_read", native_wrapper::FUNC_READ}, {"_write", native_wrapper::FUNC_WRITE}, {"_close", native_wrapper::FUNC_CLOSE},
			{"_lseek", native_wrapper::FUNC_LSEEK}, {"_stat64", native_wrapper::FUNC_STAT64}, {"malloc", native_wrapper::FUNC_MALLOC}, {"free", native_wrapper::FUNC_FREE},
			{"memcpy", native_wrapper::FUNC_MEMCPY}, {"memset", native_wrapper::FUNC_MEMSET}, {"strlen", native_wrapper::FUNC_STRLEN}, {"strcpy", native_wrapper::FUNC_STRCPY},
		};

		for (auto& f : funcs) {
			void* link = (void*)GetProcAddress(ucrt, f.name);
			if (!link) {
				printf("WARN: could not resolve %s\n", f.name);
				continue;
			}

			native_wrapper wrap;
			wrap.raw_ptr = link;
			wrap.type = f.type;

			switch (f.type) {
				case native_wrapper::FUNC_OPEN:    wrap.open = (decltype(wrap.open))link; break;
				case native_wrapper::FUNC_READ:    wrap.read = (decltype(wrap.read))link; break;
				case native_wrapper::FUNC_WRITE:   wrap.write = (decltype(wrap.write))link; break;
				case native_wrapper::FUNC_CLOSE:   wrap.close = (decltype(wrap.close))link; break;
				case native_wrapper::FUNC_LSEEK:   wrap.lseek = (decltype(wrap.lseek))link; break;
				case native_wrapper::FUNC_STAT64:  wrap.stat64 = (decltype(wrap.stat64))link; break;
				case native_wrapper::FUNC_MALLOC:  wrap.malloc = (decltype(wrap.malloc))link; break;
				case native_wrapper::FUNC_FREE:    wrap.free = (decltype(wrap.free))link; break;
				case native_wrapper::FUNC_MEMCPY:  wrap.memcpy = (decltype(wrap.memcpy))link; break;
				case native_wrapper::FUNC_MEMSET:  wrap.memset = (decltype(wrap.memset))link; break;
				case native_wrapper::FUNC_STRLEN:  wrap.strlen = (decltype(wrap.strlen))link; break;
				case native_wrapper::FUNC_STRCPY:  wrap.strcpy = (decltype(wrap.strcpy))link; break;
				default:                       break;
			}

			ucrt_table[link] = wrap;
		}
	}

	_function void vm_trap_exit() {
		// NOTE: Native functions will be resolved to ucrt_table during vm_init. When the elf is loaded .got/.plt will be linked with native functions and resolved/called from here.
		//
		uintptr_t start = vmcs->process.address; 
		uintptr_t end = start + vmcs->process.size;

		if ((vmcs->pc < start) || (vmcs->pc >= end)) {
			auto it = ucrt_table.find((void*)vmcs->pc);

			if (it == ucrt_table.end()) {
				vmcs->halt = 1;
				vmcs->reason = vm_invalid_pc;
				return;
			}

			native_wrapper& nat = it->second;
			rvm64::context::save_vm_context(); // guard against unexpected behavior

			switch (nat.type) {
				case native_wrapper::FUNC_OPEN: 
					{
						char *pathname = nullptr; int flags = 0, mode = 0;

						reg_read(char*, pathname, regenum::a0);
						reg_read(int, flags, regenum::a1);
						reg_read(int, mode, regenum::a2);

						int result = nat.open(pathname, flags, mode);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::FUNC_READ: 
					{
						int fd = 0; void *buf = nullptr; unsigned int count = 0;

						reg_read(int, fd, regenum::a0);
						reg_read(void*, buf, regenum::a1);
						reg_read(unsigned int, count, regenum::a2);

						int result = nat.read(fd, buf, count);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::FUNC_WRITE: 
					{
						int fd = 0; void* buf = nullptr; unsigned int count = 0;

						reg_read(int, fd, regenum::a0);
						reg_read(void*, buf, regenum::a1);
						reg_read(unsigned int, count, regenum::a2);

						int result = nat.write(fd, buf, count);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::FUNC_CLOSE: 
					{
						int fd = 0;
						reg_read(int, fd, regenum::a0);

						int result = nat.close(fd);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::FUNC_LSEEK: 
					{
						int fd = 0; long offset = 0; int whence = 0; 

						reg_read(int, fd, regenum::a0);
						reg_read(long, offset, regenum::a1);
						reg_read(int, whence, regenum::a2);

						long result = nat.lseek(fd, offset, whence);
						reg_write(long, regenum::a0, result);
						break;
					}
				case native_wrapper::FUNC_STAT64: 
					{
						const char *pathname = nullptr; void *statbuf = 0; 

						reg_read(const char*, pathname, regenum::a0);
						reg_read(void*, statbuf, regenum::a1);

						int result = nat.stat64(pathname, statbuf);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::FUNC_MALLOC: 
					{
						size_t size = 0; 
						reg_read(size_t, size, regenum::a0);

						void* result = nat.malloc(size);
						reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
						break;
					}
				case native_wrapper::FUNC_FREE: 
					{
						void *ptr = nullptr;
						reg_read(void*, ptr, regenum::a0);
						nat.free(ptr);
						break;
					}
				case native_wrapper::FUNC_MEMCPY: 
					{
						void *dest = nullptr; void *src = nullptr; size_t n = 0;

						reg_read(void*, dest, regenum::a0);
						reg_read(void*, src, regenum::a1);
						reg_read(size_t, n, regenum::a2);

						void* result = nat.memcpy(dest, src, n);
						reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
						break;
					}
				case native_wrapper::FUNC_MEMSET: 
					{
						void *dest = nullptr; int value = 0; size_t n = 0; 

						reg_read(void*, dest, regenum::a0);
						reg_read(int, value, regenum::a1);
						reg_read(size_t, n, regenum::a2);

						void* result = nat.memset(dest, value, n);
						reg_write(uint64_t, regenum::a0, (uint64_t)result);
						break;
					}
				case native_wrapper::FUNC_STRLEN: 
					{
						char *s = nullptr; 
						reg_read(char*, s, regenum::a0);

						size_t result = nat.strlen(s);
						reg_write(size_t, regenum::a0, result);
						break;
					}
				case native_wrapper::FUNC_STRCPY: 
					{
						char *dest = nullptr; char *src = nullptr; 

						reg_read(char*, dest, regenum::a0);
						reg_read(char*, src, regenum::a1);

						char* result = nat.strcpy(dest, src);
						reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
						break;
					}
				default: 
					{
						vmcs->halt = 1;
						vmcs->reason = vm_invalid_pc;
						break;
					}
			}
		}				

		rvm64::context::restore_vm_context();
		uintptr_t ret = 0;

		reg_read(uintptr_t, ret, regenum::ra);
		vmcs->pc = ret;
	}
}
#endif // RVNI_H
