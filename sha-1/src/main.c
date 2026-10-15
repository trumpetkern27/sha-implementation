#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha-1.h"

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage: sha-1 <input>\n");
		return 1;
	}

	char buf[64]; // should be 40 chars of message plus 2 chars leading 0x
	size_t alloc_len = strlen(argv[1]) * sizeof(byte);
	byte* msg = calloc(alloc_len / sizeof(byte), sizeof(byte)); // bc otherwise a later call to realloc won't work
	memcpy(msg, argv[1], alloc_len);
	printf("%s", sha1(msg, alloc_len, buf, sizeof(buf)));

	// input should be capped to 2^64, but srtlen can't count that high
	// if someone inputs a string that long, they must be a nation state bc they would need almost 2M TB of memory
}
