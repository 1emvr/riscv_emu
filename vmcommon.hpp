#ifndef _VMCOMMON_H
#define _VMCOMMON_H
#include <windows.h>

typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory_t)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
typedef NTSTATUS(NTAPI* NtFreeVirtualMemory_t)(HANDLE processHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType);
typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef NTSTATUS(NTAPI* NtSetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef PVOID(NTAPI* RtlAllocateHeap_t)(HANDLE HeapHandle, ULONG Flags, SIZE_T Size);

#define _native //__attribute__((section(".text$B")))
#define _vmcall //__attribute__((section(".text$B"))) __attribute__((calling_convention("custom")))
#define _naked  //__declspec(naked)

#define _rdata    __attribute__((section(".rdata")))
#define _data     __attribute__((section(".data")))
#define _extern   extern "C"

#define NT_SUCCESS(status)      ((status) >= 0)
#define NtCurrentProcess()      ((HANDLE) (LONG_PTR) -1)
#define NtCurrentThread()       ((HANDLE) (LONG_PTR) -2)

#define VM_NATIVE_STACK_ALLOC   0x210
#define VM_PROCESS_PADDING      (1024 * 256)
#define VSTACK_MAX_CAPACITY     (1024 * 1024)

#define EXPONENT_MASK           0x7FF0000000000000ULL
#define FRACTION_MASK           0x000FFFFFFFFFFFFFULL

// TODO: what happens when an non-fatal exception is raised and halt is not set?
#define CSR_SET_TRAP(epc, cause, stat, val, hlt)	\
	vmcs->csr.m_epc = (uintptr_t)epc;		\
	vmcs->csr.m_cause = cause;				\
	vmcs->csr.m_status = stat;				\
	vmcs->csr.m_tval = val;					\
	vmcs->halt = hlt;						\
	__debugbreak()

#ifdef DEBUG
#define CSR_GET(ctx_ptr)						\
	do {										\
	uintptr_t csr1 = vmcs->csr.m_epc;			\
	uintptr_t csr2 = vmcs->csr.m_cause; 		\
	uintptr_t csr3 = vmcs->csr.m_status;		\
	uintptr_t csr4 = vmcs->csr.m_tval;			\
	uintptr_t ip = ctx_ptr->ContextRecord->Rip; \
	__debugbreak();								\
} while (0)
#else
#define CSR_GET(ctx_ptr)						\
	do {										\
	uintptr_t csr1 = vmcs->csr.m_epc;			\
	uintptr_t csr2 = vmcs->csr.m_cause; 		\
	uintptr_t csr3 = vmcs->csr.m_status;		\
	uintptr_t csr4 = vmcs->csr.m_tval;			\
	uintptr_t ip = ctx_ptr->ContextRecord->Rip; \
} while (0)
#endif

#define SAVE_VM_CONTEXT(expr)	\
	save_vm_context();			\
	expr;						\
	restore_vm_context()

enum causenum {
	supervisor_software_interrupt = 0xb11,
	machine_software_interrupt = 0xb13,
	supervisor_timer_interrupt = 0xb15,
	machine_timer_interrupt = 0xb17,
	supervisor_external_interrupt = 0xb19,
	machine_external_interrupt = 0xb111,
	designated_for_platform_use = 0xb116,
	instruction_address_misaligned = 0xb00,
	instruction_access_fault = 0xb01,
	illegal_instruction = 0xb02,
	breakpoint = 0xb03,
	load_address_misaligned = 0xb04,
	load_access_fault = 0xb05,
	store_amo_address_misaligned = 0xb06,
	store_amo_access_fault = 0xb07,
	environment_call_from_u_mode = 0xb08,
	environment_call_from_s_mode = 0xb09,
	environment_call_from_m_mode = 0xb011,
	instruction_page_fault = 0xb012,
	load_page_fault = 0xb013,
	store_amo_page_fault = 0xb015,
	environment_call_native = 0xb024,
	bad_image_symbol = 0xb025,
	bad_image_load = 0xb026,
	bad_image_type = 0xb027,
	designated_for_custom_use_5 = 0xb028,
	designated_for_custom_use_6 = 0xb029,
	designated_for_custom_use_7 = 0xb030,
	designated_for_custom_use_8 = 0xb031,
	undefined = 0xffff,
};

enum screnum {
    rd, rs1, rs2, rs3, imm,
};

enum regenum {
	zr, ra, sp, gp, tp,
	t0, t1, t2, s0, s1,
	a0, a1, a2, a3, a4, a5, a6, a7,
	s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
	t3, t4, t5, t6,
};

enum typenum {
	rtype = 1, r4type, itype, stype, btype, utype, jtype,
};

_extern {
void save_host_context();
void restore_host_context();
void save_vm_context();
void restore_vm_context();
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

	_data simple_map::unordered_map<uintptr_t, native_wrapper> *ucrt_native_table;

	vmcall native_wrapper second() {
		native_wrapper found = simple_map::find(ucrt_native_table, (uintptr_t)0x1234);
		return found;
	}

	vmcall int main() {
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


#endif // _VMCOMMON_H
