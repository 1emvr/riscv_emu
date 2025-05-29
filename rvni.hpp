#ifndef RVNI_H
#define RVNI_H
#include <unordered_map>

#include "vmmain.hpp"
#include "vmctx.hpp"

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
	__data unordered_map::table_map *ucrt_native_table;

	namespace unordered_map {
		struct entry {
			uintptr_t address;		
			native_wrapper wrapper;
		};

		struct table_map {
			entry *entries;
			size_t capacity;
		};

		table_map* init() {
			table_map *map = (table_map*)malloc(sizeof(table_map));
			map->entries = nullptr;
			map->capacity = 0;
			return map;
		}

		static bool end() {
			return false;
		}

		void push(table_map *map, uintptr_t address, native_wrapper wrapper) {
			entry *temp = (entry*) realloc(map->entries, sizeof(entry) * (map->capacity + 1));
			if (!temp) {
				return;
			}
			map->entries = temp;
			map->entries[map->capacity].address = address; 
			map->entries[map->capacity].wrapper = wrapper; 
			map->capacity += 1;
		}

		native_wrapper find(table_map *map, uintptr_t k) {
			for (size_t i = 0; i < map->capacity; i++) {
				if (map->entries[i].address == k) {
					return map->entries[i].wrapper;	
				}
			}
			return native_wrapper{ };
		}

		void pop(table_map *map, uintptr_t k) {
			for (size_t i = 0; i < map->capacity; i++) {
				if (map->entries[i].address == k) {
					for (size_t j = 0; j < map->capacity - 1; j++) {
						map->entries[j] = map->entries[j + 1];
					}
					map->capacity -= 1;
					if (map->capcity > 0) {
						map->entries = (entry*) realloc(map->entries, sizeof(entry) * map->capacity);
					} else {
						free(map->entries);
						map->entries = nullptr;
					}
					return;
				}
			}
		}

		void destroy(table_map *map) {
			if (map) {
				free(map->entries);
				free(map);
			}
		}
	}

	int main() {
		void *new_address = 0x1234;
		native_wrapper new_wrapper = { };

		ucrt_native_table = unordered_map::init();
		unordered_map::push(&ucrt_native_table, (uintpr_t)new_address, new_wrapper);

		native_wrapper found = unordered_map::find(&ucrt_native_table, (uintptr_t)0x1234);
		unordered_map::destroy(&ucrt_native_table);

		return 0;
	} 
	*/

	__data std::unordered_map<void*, native_wrapper> ucrt_native_table; // NOTE: might cause compiler exception (unsure)
																
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
		if ((vmcs->pc >= vmcs->plt.start) && (vmcs->pc < vmcs->plt.end)) {
			auto it = ucrt_native_table.find((void*)vmcs->pc); 

			if (it == ucrt_native_table.end()) {
				vmcs->csr.m_cause = illegal_operation;                                
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
						reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
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
						reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
						break;
					}
				case native_wrapper::PLT_MEMSET: 
					{
						void *dest; int value = 0; size_t n = 0; 

						reg_read(void*, dest, regenum::a0);
						reg_read(int, value, regenum::a1);
						reg_read(size_t, n, regenum::a2);

						void* result = plt.memset(dest, value, n);
						reg_write(uint64_t, regenum::a0, (uint64_t)result);
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
						reg_write(uintptr_t, regenum::a0, (uintptr_t)result);
						break;
					}
				default: 
					{
						vmcs->csr.m_cause = illegal_operation;                                
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
