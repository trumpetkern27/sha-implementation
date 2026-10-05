#include <stddef.h>

#ifndef SHA_1_H
#define SHA_1_H
#endif

typedef unsigned char byte;
#define WORD_BIT_LENGTH 8 * 4
#define WORD_BYTE_LENGTH 4
#define MSG_LENGTH_MULTIPLE 512
typedef byte block[4][4]; // 4 x 4 matrix of bytes
typedef byte word[WORD_BYTE_LENGTH]; // 4 bytes


/* rotates x leftwards n times */
void rotl_n(size_t n, word x, word* out);
/* rotates x rightwards n times */
void rotr_n(size_t n, word x, word* out);

// 'sha-1 functions'
void ch(word x, word y, word z, word* out);
void parity(word x, word y, word z, word* out);
void maj(word x, word y, word z, word* out);
void f_t(short t, word x, word y, word z, word* out);

// sha constant
const byte* k_t(short t); // use byte*, functionally equivalent to returning word but C hates arrays

void pad_msg(byte* msg, size_t msg_bit_length);
