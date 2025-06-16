#ifndef VMRWX_HPP
#define VMRWX_HPP
#define unwrap_opcall(hdl_idx) 									\
	do { 														\
		auto a = ((uintptr_t*)vmcs->handler)[hdl_idx];			\
		auto b = rvm64::crypt::decrypt_ptr((uintptr_t)a);		\
		void (*fn)() = (void (*)())(b);							\
		fn();													\
	} while(0)

#define mem_read(T, retval, addr)  								\
	do {														\
		retval = *(T *)(vmcs->process.address +  				\
				((addr) - vmcs->process.address)); 				\
	} while(0)

#define mem_write(T, addr, value)  								\
	do {														\
		*(T *)(vmcs->process.address +  						\
				((addr) - vmcs->process.address)) = value;  	\
	} while(0)

#define reg_read(T, dst, reg_idx) 								\
	do { 														\
		dst = (T)vmcs->vregs[(reg_idx)];						\
	} while(0)

#define reg_write(T, reg_idx, src) 								\
	do { 														\
		vmcs->vregs[(reg_idx)] = (T)(src);						\
	} while(0)

#define scr_read(T, dst, scr_idx) 								\
	do { 														\
		dst = (T)vmcs->vscratch[(scr_idx)];						\
	} while(0)

#define scr_write(T, scr_idx, src) 								\
	do { 														\
		vmcs->vscratch[(scr_idx)] = (T)(src);					\
	} while(0)

#endif // VMRWX_HPP
