#include <stdio.h>
#include <string.h>

unsigned long long long_strlen(char* s) {
	unsigned long long i;
	while (*s != '\0')
		i++;
	return i;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage: sha-1 <input>\n");
		return 1;
	}
	// input should be capped to 2^64, but srtlen can't count that high
	// if someone inputs a string that long, they must be a nation state bc they would need almost 2M TB of memory


}
