#include <cstring>

extern "C" void* malloc(size_t size);
extern "C" void free(void *ptr);
extern "C" void* memcpy(void* dst, void* src, size_t num);
extern "C" int printf (const char * format, ...);

extern "C" int main() {
	int result = 0xFFFF;
	void *buffer = malloc(sizeof(int));

	memcpy(buffer, &result, sizeof(int));
	memset(buffer, 0, sizeof(int));
	free(buffer);

	return 0;
}
