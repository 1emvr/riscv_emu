#ifndef RVNI_H
#define RVNI_H
#include "vmmain.h"

namespace rvm64::rvni {
	struct native_wrapper {
		void* raw_ptr;  // raw function pointer
		enum FuncType {
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

	__data std::unordered_map<void*, native_wrapper> ucrt_table; // NOTE: might cause compiler exception (unsure)

	void resolve_ucrt_imports() {
		HMODULE ucrt = LoadLibraryA("ucrtbase.dll");

		if (!ucrt) {
			printf("ERROR: Failed to load ucrtbase.dll\n");
			return;
		}

		struct {
			const char* name;
			native_wrapper::FuncType type;
		} funcs[] = {
			{"_open", native_wrapper::FUNC_OPEN}, {"_read", native_wrapper::FUNC_READ}, {"_write", native_wrapper::FUNC_WRITE}, {"_close", native_wrapper::FUNC_CLOSE},
			{"_lseek", native_wrapper::FUNC_LSEEK}, {"_stat64", native_wrapper::FUNC_STAT64}, {"malloc", native_wrapper::FUNC_MALLOC}, {"free", native_wrapper::FUNC_FREE},
			{"memcpy", native_wrapper::FUNC_MEMCPY}, {"memset", native_wrapper::FUNC_MEMSET}, {"strlen", native_wrapper::FUNC_STRLEN}, {"strcpy", native_wrapper::FUNC_STRCPY}
		};

		for (auto& f : funcs) {
			void* addr = (void*)GetProcAddress(ucrt, f.name);
			if (!addr) {
				printf("WARN: could not resolve %s\n", f.name);
				continue;
			}

			native_wrapper wrap;
			wrap.raw_ptr = addr;
			wrap.type = f.type;

			switch (f.type) {
				case native_wrapper::FUNC_OPEN:    wrap.open = (decltype(wrap.open))addr; break;
				case native_wrapper::FUNC_READ:    wrap.read = (decltype(wrap.read))addr; break;
				case native_wrapper::FUNC_WRITE:   wrap.write = (decltype(wrap.write))addr; break;
				case native_wrapper::FUNC_CLOSE:   wrap.close = (decltype(wrap.close))addr; break;
				case native_wrapper::FUNC_LSEEK:   wrap.lseek = (decltype(wrap.lseek))addr; break;
				case native_wrapper::FUNC_STAT64:  wrap.stat64 = (decltype(wrap.stat64))addr; break;
				case native_wrapper::FUNC_MALLOC:  wrap.malloc = (decltype(wrap.malloc))addr; break;
				case native_wrapper::FUNC_FREE:    wrap.free = (decltype(wrap.free))addr; break;
				case native_wrapper::FUNC_MEMCPY:  wrap.memcpy = (decltype(wrap.memcpy))addr; break;
				case native_wrapper::FUNC_MEMSET:  wrap.memset = (decltype(wrap.memset))addr; break;
				case native_wrapper::FUNC_STRLEN:  wrap.strlen = (decltype(wrap.strlen))addr; break;
				case native_wrapper::FUNC_STRCPY:  wrap.strcpy = (decltype(wrap.strcpy))addr; break;
				default:                       break;
			}

			ucrt_table[addr] = wrap;
		}
	}

	__function bool vm_trap_exit() {
		//
		uintptr_t start = vmcs->process.address; 
		uintptr_t end = start + vmcs->process.size;

		if ((vmcs->pc < start) || (vmcs->pc >= end)) {
			// NOTE: native functions reside in ucrt_table. when the elf is loaded .got/.plt will be linked with native functions
			
			void* pc_ptr = (void*)vmcs->pc;
			auto it = ucrt_table.find(pc_ptr);

			if (it == native_func_table.end()) {
				vmcs->halt = 1;
				vmcs->reason = vm_invalid_pc;
				return false;
			}

			native_wrapper& nat = it->second;
			switch (nat.type) {
				case native_func::FUNC_OPEN: {
												 const char *pathname = nullptr; int flags = 0, mode = 0;

												 reg_read(const char*, pathname, regenum::a0);
												 reg_read(int, flags, regenum::a1);
												 reg_read(int, mode, regenum::a2);

												 int result = nat.open(pathname, flags, mode);
												 reg_write(int, regenum::a0, result);
												 break;
											 }
				case native_func::FUNC_READ: {
												 int fd = 0; void *buf = nullptr; unsigned int count = 0;

												 reg_read(int, fd, regenum::a0);
												 reg_read(void*, buf, regenum::a1);
												 reg_read(unsigned int, count, regenum::a2);

												 int result = nat.read(fd, buf, count);
												 reg_write(int, regenum::a0, result);
												 break;
											 }
				case native_func::FUNC_WRITE: {
												  int fd = 0; void* buf = nullptr; unsigned int count = 0;

												  reg_read(int, fd, regenum::a0);
												  reg_read(void*, buf, regenum::a1);
												  reg_read(unsigned int, count, regenum::a2);

												  int result = nat.write(fd, buf, count);
												  reg_write(int, regenum::a0, result);
												  break;
											  }
				case native_func::FUNC_CLOSE: {
												  int fd = 0;
												  reg_read(int, fd, regenum::a0);

												  int result = nat.close(fd);
												  reg_write(int, regenum::a0, result);
												  break;
											  }
				case native_func::FUNC_LSEEK: {
												  int fd = 0; long offset = 0; int whence = 0; 

												  reg_read(int, fd, regenum::a0);
												  reg_read(long, offset, regenum::a1);
												  reg_read(int, whence, regenum::a2);

												  long result = nat.lseek(fd, offset, whence);
												  reg_write(long, regenum::a0, result);
												  break;
											  }
				case native_func::FUNC_STAT64: {
												   const char *pathname = nullptr; void *statbuf = 0; 

												   reg_read(const char*, pathname, regenum::a0);
												   reg_read(void*, statbuf, regenum::a1);

												   int result = nat.stat64(pathname, statbuf);
												   reg_write(int, regenum::a0, result);
												   break;
											   }
				case native_func::FUNC_MALLOC: {
												   size_t size = 0; 
												   reg_read(size_t, size, regenum::a0);

												   void* result = nat.malloc(size);
												   reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
												   break;
											   }
				case native_func::FUNC_FREE: {
												 void *ptr = nullptr;
												 reg_read(void*, ptr, regenum::a0);
												 nat.free(ptr);
												 break;
											 }
				case native_func::FUNC_MEMCPY: {
												   void *dest = nullptr; void *src = nullptr; size_t n = 0;

												   reg_read(void*, dest, regenum::a0);
												   reg_read(void*, src, regenum::a1);
												   reg_read(size_t, n, regenum::a2);

												   void* result = nat.memcpy(dest, src, n);
												   reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
												   break;
											   }
				case native_func::FUNC_MEMSET: {
												   void *dest = nullptr; int value = 0; size_t n = 0; 

												   reg_read(void*, dest, regenum::a0);
												   reg_read(int, value, regenum::a1);
												   reg_read(size_t, n, regenum::a2);

												   void* result = nat.memset(dest, value, n);
												   reg_write(uint64_t, regenum::a0, (uint64_t)result);
												   break;
											   }
				case native_func::FUNC_STRLEN: {
												   char *s = nullptr; 
												   reg_read(char *s, s, regenum::a0);

												   size_t result = nat.strlen(s);
												   reg_write(size_t, regenum::a0, result);
												   break;
											   }
				case native_func::FUNC_STRCPY: {
												   char *dest = nullptr; char *src = nullptr; 

												   reg_read(char*, dest, regenum::a0);
												   reg_read(char*, src, regenum::a1);

												   char* result = nat.strcpy(dest, src);
												   reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
												   break;
											   }
				default: {
							 printf("WARN: Unknown native function type: %d\n", nat.type);
							 vmcs->halt = 1;
							 vmcs->reason = vm_invalid_pc;
							 break;
						 }
			}
		}				

		return false;
	}
}
#endif // RVNI_H
