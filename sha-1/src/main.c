#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "sha-1.h"

int main(int argc, char** argv) {
	char* buf;
	size_t alloc_len;

	if (argc > 1) {
		alloc_len = strlen(argv[1]);
		buf = calloc(alloc_len, sizeof(char));
		memcpy(buf, argv[1], alloc_len);
	} else if (!isatty(fileno(stdin))) { // handle piped input
		int c;
		size_t i = 0, sz = 64;
		char* tmp;
		buf = calloc(sz, sizeof(char));
		while (true) {
			c = getchar();
			if (c == EOF) break;
			buf[i++] = c;
			if (i >= sz) {
				tmp = calloc(sz * 2, sizeof(char));
				memcpy(tmp, buf, sz);
				free(buf);
				buf = tmp;
				sz *= 2;
			}
		}
		buf[i--] = '\0';
		alloc_len = i;
	} else {
		printf("Usage: sha-1 <input>\n");
		_exit(1);
	}

	byte* msg = calloc(alloc_len / sizeof(byte), sizeof(byte)); // bc otherwise a later call to realloc won't work
	memcpy(msg, buf, alloc_len);
	char out[64];
	printf("%s", sha1(msg, alloc_len, out, sizeof(out)));

	// input should be capped to 2^64, but srtlen can't count that high
	// if someone inputs a string that long, they must be a nation state bc they would need almost 2M TB of memory
}
