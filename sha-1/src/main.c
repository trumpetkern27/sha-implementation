#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "sha-1.h"

struct read_stdin_args { char* buf; size_t sz; size_t length; };
void* read_stdin(void* arguments);
void* check_stdin(void* vbuf);

int main(int argc, char** argv) {
	char* buf;
	size_t alloc_len;

	if (argc > 1) {
		alloc_len = strlen(argv[1]);
		buf = calloc(alloc_len, sizeof(char));
		memcpy(buf, argv[1], alloc_len);
	} else { // handle piped input
		// this is so freaking scuffed, idk how else to do this gracefully
		buf = calloc(64, sizeof(char));
		struct read_stdin_args args = {
			.buf = buf,
			.sz = 64
		};
		pthread_t reader, watcher;
		pthread_create(&reader, NULL, read_stdin, (void*)&args);
		pthread_create(&watcher, NULL, check_stdin, (void*)&buf);
		char* tmp;
		pthread_join(watcher, (void**)&tmp);
		pthread_join(reader, NULL);
		alloc_len = args.length;
		buf = args.buf; // in case it needed to realloc
	}

	byte* msg = calloc(alloc_len / sizeof(byte), sizeof(byte)); // bc otherwise a later call to realloc won't work
	memcpy(msg, buf, alloc_len);
	char out[64];
	printf("%s", sha1(msg, alloc_len, out, sizeof(out)));

	// input should be capped to 2^64, but srtlen can't count that high
	// if someone inputs a string that long, they must be a nation state bc they would need almost 2M TB of memory
}

void* read_stdin(void* arguments) {
	struct read_stdin_args* args = arguments;
	size_t i = 0;
	char* tmp;
	int c;
	while (true) {
		c = getchar();
		if (c == EOF) break;
		(args->buf)[i++] = c;
		if (i >= args->sz) {
			tmp = calloc(args->sz * 2, sizeof(char));
			memcpy(tmp, args->buf, args->sz);
			free(args->buf);
			args->buf = tmp;
			args->sz *= 2;
		}
	}
	(args->buf)[i--] = '\0'; // USUALLY outputs from other processes will end in a newline char instead of a null char
							 // so we replace its terminal char with a null char
	args->length = i;
	return NULL;
}

// checks to see if stdin has been written to, 50ms delay
// should be fast enough to not be noticeable but slow enough to receive input from a piped output
void* check_stdin(void* vbuf) {
	char** buf = (char**) vbuf;
	usleep(50000);
	if ((*buf)[0] == '\0') {
		printf("Usage: sha-1 <input>\n");
		_exit(1);
	}
	return NULL;
}
