#include <stddef.h>

extern "C" void* malloc(size_t size);
extern "C" void* memcpy(void* dst, void* src, size_t num);
extern "C" int printf (const char * format, ...);

extern "C" int main() {
	int result = 0xFFFF;
	void *buffer = malloc(sizeof(int));

	memcpy(buffer, &result, sizeof(int));
	printf("the result is 0x%lx\n", *(int*)buffer);
	return 0;
}
