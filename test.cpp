#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main() {
	int result = 0xFFFF;
	void *buffer = malloc(sizeof(int));

	memcpy(buffer, (void*)&result, sizeof(int));
	printf("the result is 0x%lx\n", buffer);
	return 0;
}
