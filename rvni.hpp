#ifndef RVNI_H
#define RVNI_H
#include <unordered_map>
#include <windows.h>

#include "vmmain.hpp"
#include "vmcommon.hpp"
#include "vmmem.hpp"

namespace rvm64::rvni {
	struct native_wrapper {
		void *address;  

		enum typecaster {
			PLT_OPEN, PLT_READ, PLT_WRITE, PLT_CLOSE,
			PLT_LSEEK, PLT_STAT64, PLT_MALLOC, PLT_FREE,
			PLT_MEMCPY, PLT_MEMSET, PLT_STRLEN, PLT_STRCPY,
			PLT_UNKNOWN
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

	/*
	
	#define malloc(x) 		ctx->win32.RtlAllocateHeap(ctx->heap, 0, x);
	#define realloc(p, x) 	ctx->win32.RtlReAllocateHeap(ctx->heap, 0, p, x);
	#define free(x) 		ctx->win32.RtlFreeHeap(ctx->heap, 0, x);

	namespace simple_map {
		template<typename A, typename B>
		struct entry {
			A key;		
			B value;
		};

		template<typename A, typename B>
		struct unordered_map {
			entry<A, B> *entries;
			size_t capacity;
		};

		template <typename A, typename B>
		simple_map::unordered_map<A, B>* init() {

			simple_map::unordered_map<A, B> *map = 
				(simple_map::unordered_map<A, B>*) malloc(sizeof(simple_map::unordered_map<A, B>));

			map->entries = nullptr;
			map->capacity = 0;
			return map;
		}

		template <typename A, typename B>
		void push(simple_map::unordered_map<A, B> *map, A key, B value) {
			entry<A, B> *temp = (entry<A, B>*) realloc(map->entries, sizeof(entry<A, B>) * (map->capacity + 1));

			if (!temp) {
				return;
			}
			map->entries = temp;
			map->entries[map->capacity].key = key; 
			map->entries[map->capacity].value = value; 
			map->capacity += 1;
		}

		template <typename A, typename B>
		B find(simple_map::unordered_map<A, B> *map, A key) {

			for (size_t i = 0; i < map->capacity; i++) {
				if (map->entries[i].key == key) {
					return map->entries[i].value;	
				}
			}
			return B{ };
		}

		template <typename A, typename B>
		void pop(simple_map::unordered_map<A, B> *map, A key) {

			for (size_t i = 0; i < map->capacity; i++) {
				if (map->entries[i].key == key) {

					for (size_t j = i; j < map->capacity - 1; j++) { // unlink from the list
						map->entries[j] = map->entries[j + 1];
					}
					map->capacity -= 1;

					if (map->capacity > 0) {
						map->entries = (entry<A, B>*) realloc(map->entries, sizeof(entry<A, B>) * map->capacity);
					} else {
						free(map->entries);
						map->entries = nullptr;
					}
					return;
				}
			}
		}

		template <typename A, typename B>
		void destroy(simple_map::unordered_map<A, B> *map) {
			if (map) {
				free(map->entries);
				free(map);
			}
		}
	}

	__data simple_map::unordered_map<uintptr_t, native_wrapper> *ucrt_native_table;

	__vmcall native_wrapper second() {
		native_wrapper found = simple_map::find(ucrt_native_table, (uintptr_t)0x1234);
		return found;
	}

	__vmcall int main() {
		void *new_address = 0x1234;
		native_wrapper new_wrapper = { 
			.address = new_address;
			.open = decltype(open);
		};

		ucrt_native_table = simple_map::init<uintptr_t, native_wrapper>();
		simple_map::push(ucrt_native_table, (uintptr_t)new_address, new_wrapper);

		auto found = second();
		simple_map::destroy(ucrt_native_table);

		if (found.address == 0) {
			return 1;
		}

		return 0;
	} 
	*/

	__data std::unordered_map<void*, native_wrapper> ucrt_native_table; 
																
	struct ucrt_alias {
		const char* original;
		const char* alias;
	};

	__data const ucrt_alias alias_table[] = {
		{ "open",  "_open"  }, { "read",  "_read"  }, { "write", "_write" },
		{ "close", "_close" }, { "exit",  "_exit"  },
	};


	__native void* windows_thunk_resolver(const char* sym_name) {
		static HMODULE ucrt = LoadLibraryA("ucrtbase.dll");
		if (!ucrt) {
			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			return nullptr;
		}

		for (size_t i = 0; i < sizeof(alias_table) / sizeof(alias_table[0]); ++i) {
			if (strcmp(sym_name, alias_table[i].original) == 0) {
				sym_name = alias_table[i].alias;
				break;
			}
		}

		void* proc = (void*)GetProcAddress(ucrt, sym_name);
		if (!proc) {
			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			return nullptr;
		}

		return proc;
	}

	__native void resolve_ucrt_imports() {
		HMODULE ucrt = LoadLibraryA("ucrtbase.dll");

		if (!ucrt) {
			printf("ERR: could not load ucrtbase.dll\n");

			vmcs->halt = 1;
			vmcs->csr.m_cause = vm_undefined;
			return;
		}

		struct {
			const char* name;
			native_wrapper::typecaster type;
		} funcs[] = {
			{"_open", native_wrapper::PLT_OPEN}, {"_read", native_wrapper::PLT_READ}, {"_write", native_wrapper::PLT_WRITE}, {"_close", native_wrapper::PLT_CLOSE},
			{"_lseek", native_wrapper::PLT_LSEEK}, {"_stat64", native_wrapper::PLT_STAT64}, {"malloc", native_wrapper::PLT_MALLOC}, {"free", native_wrapper::PLT_FREE},
			{"memcpy", native_wrapper::PLT_MEMCPY}, {"memset", native_wrapper::PLT_MEMSET}, {"strlen", native_wrapper::PLT_STRLEN}, {"strcpy", native_wrapper::PLT_STRCPY},
		};

		for (auto& f : funcs) {
			void* native = (void*)GetProcAddress(ucrt, f.name);
			if (!native) {
				printf("ERR: could not resolve %s\n", f.name);

				vmcs->halt = 1;
				vmcs->csr.m_cause = vm_undefined;
				return;
			}

			native_wrapper wrap;
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
				default:                       
												  {
													  vmcs->halt = 1;
													  vmcs->csr.m_cause = vm_undefined;
													  return;
												  }
			}

			ucrt_native_table[native] = wrap;
		}
	}

	__native void vm_trap_exit() {
		// case VM_NATIVE_CALL:
		if ((vmcs->pc >= vmcs->process.plt.start) && (vmcs->pc < vmcs->process.plt.end)) {
			auto it = ucrt_native_table.find((void*)vmcs->pc); 

			if (it == ucrt_native_table.end()) {
				vmcs->csr.m_cause = illegal_instruction;                                
				vmcs->csr.m_epc = vmcs->pc;                                           
				vmcs->csr.m_tval = vmcs->pc;                                            
				vmcs->halt = 1;
			}

			native_wrapper& plt = it->second;

			switch (plt.type) {
				case native_wrapper::PLT_OPEN: 
					{
						char *pathname; int flags = 0, mode = 0;

						reg_read(char*, pathname, regenum::a0);
						reg_read(int, flags, regenum::a1);
						reg_read(int, mode, regenum::a2);

						int result = plt.open(pathname, flags, mode);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_READ: 
					{
						int fd = 0; void *buf; unsigned int count = 0;

						reg_read(int, fd, regenum::a0);
						reg_read(void*, buf, regenum::a1);
						reg_read(unsigned int, count, regenum::a2);

						int result = plt.read(fd, buf, count);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_WRITE: 
					{
						int fd = 0; void *buf; unsigned int count = 0;

						reg_read(int, fd, regenum::a0);
						reg_read(void*, buf, regenum::a1);
						reg_read(unsigned int, count, regenum::a2);

						int result = plt.write(fd, buf, count);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_CLOSE: 
					{
						int fd = 0;
						reg_read(int, fd, regenum::a0);

						int result = plt.close(fd);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_LSEEK: 
					{
						int fd = 0; long offset = 0; int whence = 0; 

						reg_read(int, fd, regenum::a0);
						reg_read(long, offset, regenum::a1);
						reg_read(int, whence, regenum::a2);

						long result = plt.lseek(fd, offset, whence);
						reg_write(long, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_STAT64: 
					{
						const char *pathname; void *statbuf; 

						reg_read(const char*, pathname, regenum::a0);
						reg_read(void*, statbuf, regenum::a1);

						int result = plt.stat64(pathname, statbuf);
						reg_write(int, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_MALLOC: 
					{
						size_t size = 0; 
						reg_read(size_t, size, regenum::a0);

						void* result = plt.malloc(size);
						reg_write(uintptr_t, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_FREE: 
					{
						void *ptr;
						reg_read(void*, ptr, regenum::a0);
						plt.free(ptr);
						break;
					}
				case native_wrapper::PLT_MEMCPY: 
					{
						void *dest, *src; size_t n = 0;

						reg_read(void*, dest, regenum::a0);
						reg_read(void*, src, regenum::a1);
						reg_read(size_t, n, regenum::a2);

						void* result = plt.memcpy(dest, src, n);
						reg_write(uintptr_t, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_MEMSET: 
					{
						void *dest; int value = 0; size_t n = 0; 

						reg_read(void*, dest, regenum::a0);
						reg_read(int, value, regenum::a1);
						reg_read(size_t, n, regenum::a2);

						void* result = plt.memset(dest, value, n);
						reg_write(uint64_t, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_STRLEN: 
					{
						char *s; 
						reg_read(char*, s, regenum::a0);

						size_t result = plt.strlen(s);
						reg_write(size_t, regenum::a0, result);
						break;
					}
				case native_wrapper::PLT_STRCPY: 
					{
						char *dest, *src; 

						reg_read(char*, dest, regenum::a0);
						reg_read(char*, src, regenum::a1);

						char* result = plt.strcpy(dest, src);
						reg_write(uintptr_t, regenum::a0, result);
						break;
					}
				default: 
					{
						vmcs->csr.m_cause = illegal_instruction;                                
						vmcs->csr.m_epc = vmcs->pc;                                           
						vmcs->csr.m_tval = (plt.type);                                            
						vmcs->halt = 1;
					}
			}
		} else {
			vmcs->csr.m_cause = instruction_access_fault;                                
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = vmcs->pc;                                            
			vmcs->halt = 1;
		}				

		uintptr_t ret = 0;
		reg_read(uintptr_t, ret, regenum::ra);

		vmcs->csr.m_cause = 0;
		vmcs->step = true;
		vmcs->pc = ret; 
	}
}
#endif // RVNI_H
