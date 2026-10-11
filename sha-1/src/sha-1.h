#include <stddef.h>
#include <stdint.h>

#ifndef SHA_1_H
#define SHA_1_H
#endif

typedef unsigned char byte;
#define WORD_BIT_LENGTH 8 * 4
#define WORD_BYTE_LENGTH 4
#define MSG_LENGTH_MULTIPLE 512
#define MSG_BYTE_LENGTH 64 // length of message block in bytes
typedef byte block[4][4]; // 4 x 4 matrix of bytes
typedef byte word[WORD_BYTE_LENGTH]; // 4 bytes


/* rotates x leftwards n times */
word* rotl_n(size_t n, word x, word* out);
/* rotates x rightwards n times */
word* rotr_n(size_t n, word x, word* out);

// 'sha-1 functions'
word* ch(word x, word y, word z, word* out);
word* parity(word x, word y, word z, word* out);
word* maj(word x, word y, word z, word* out);
word* f_t(short t, word x, word y, word z, word* out);

// sha constant
byte* k_t(short t); // use byte*, functionally equivalent to returning word but C hates arrays

// message
uint64_t pad_msg(byte* msg, size_t msg_bit_length);
word* w_t(word* m, short t, word* out);

// algo
char* sha1(byte* msg, char* output, size_t len);
