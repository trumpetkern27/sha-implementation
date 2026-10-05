#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha-1.h"

const char* byte_to_binary(int x);

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Usage: sha-1 <input>\n");
		return 1;
	}
	word w;
	char* p;
	for (size_t i = 0; i < 4; i++) {
		w[i] = strtoul(argv[i + 1], &p, 2);
		if (*p != '\0')
			printf("wrong: %s\n", p);
	}
	word out;
	rotr_n(7, w, &out);

	for (size_t i = 0; i < 4; i++) {
		printf("%s ", byte_to_binary(out[i]));
	}
	printf("\n");
	// input should be capped to 2^64, but srtlen can't count that high
	// if someone inputs a string that long, they must be a nation state bc they would need almost 2M TB of memory


}

const char* byte_to_binary(int x) {
	static char b[9];
	b[0] = '\0';
	int z;
	for (z = 128; z > 0; z >>=1 )
		strcat(b, ((x & z) == z) ? "1" : "0");
	return b;
}
