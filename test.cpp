#include <string.h>
#include <stdlib.h>

// NOTE: do not mangle "main"
extern "C" int main() {
	constexpr int result = 0xFFFF;
	void *buffer = malloc(sizeof(int));

	memcpy(buffer, &result, sizeof(int));
	memset(buffer, 0, sizeof(int));
	free(buffer);

	return 0;
}
