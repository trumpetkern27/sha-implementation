#include "sha-1.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// SECTION: glob

static word* g_k_words;
static word* g_h_0;

// SECTION: helpers

// converts word to uint32_t in big-endian
// helpful bc word here will always 4 bytes = 32 bits
uint32_t word_to_uint32_t_be(word x) {
	return ((uint32_t)x[0] << 24) |
		((uint32_t)x[1] << 16) |
		((uint32_t)x[2] << 8) |
		(uint32_t)x[3];
}

// converts uint32_t to word in big-endian
word* uint32_t_to_word_be(uint32_t i, word* out) {
	(*out)[0] = (byte)(i >> 24) & 0xFF;
	(*out)[1] = (byte)(i >> 16) & 0xFF;
	(*out)[2] = (byte)(i >> 8) & 0xFF;
	(*out)[3] = (byte)i & 0xFF;
	return out;
}

void throw_bad_t_val(short t) {
	printf("bad value for t (%d)\n", t);
	exit(1);
}

// SECTION: bitwise functions

word* rotl_n(size_t n, word x, word* out) {
	uint32_t x_32 = word_to_uint32_t_be(x); // convert to uint32_t to make life easier
	uint32_t_to_word_be((x_32 << n) | (x_32 >> (WORD_BIT_LENGTH - n)), out);
	return out;
}

word* rotr_n(size_t n, word x, word* out) {
	uint32_t x_32 = word_to_uint32_t_be(x);
	uint32_t_to_word_be((x_32 >> n) | (x_32 << (WORD_BIT_LENGTH - n)), out);
	return out;
}

// SECTION: sha-1 functions
// defined per FIPS 180-4, 4.1.1

word* ch(word x, word y, word z, word* out) {
	uint32_t x_32 = word_to_uint32_t_be(x);
	uint32_t y_32 = word_to_uint32_t_be(y);
	uint32_t z_32 = word_to_uint32_t_be(z);
	return uint32_t_to_word_be(
		// function
		(x_32 & y_32) ^ (~x_32 & z_32),
	out);
}

word* parity(word x, word y, word z, word* out) {
	uint32_t x_32 = word_to_uint32_t_be(x);
	uint32_t y_32 = word_to_uint32_t_be(y);
	uint32_t z_32 = word_to_uint32_t_be(z);
	return uint32_t_to_word_be(
		// function
		x_32 ^ y_32 ^ z_32,
	out);
}

word* maj(word x, word y, word z, word* out) {
	uint32_t x_32 = word_to_uint32_t_be(x);
	uint32_t y_32 = word_to_uint32_t_be(y);
	uint32_t z_32 = word_to_uint32_t_be(z);
	return uint32_t_to_word_be(
		// function
		(x_32 & y_32) ^ (x_32 & z_32) ^ (y_32 & z_32),
	out);
}

word* f_t(short t, word x, word y, word z, word* out) {
	if (t < 0 || t >= 80)
		throw_bad_t_val(t);

	// def
	if (0 <= t && t <= 19)
		return ch(x, y, z, out);
	else if (20 <= t && t <= 39)
		return parity(x, y, z, out);
	else if (40 <= t && t <= 59)
		return maj(x, y, z, out);
	else if (60 <= t && t <= 79)
		return parity(x, y, z, out);
}

// pads message as follows:
// 	find k for msg_bit_length + 1 + k \equiv 448 mod 512 for the smallest non-negative k
// 	append the message with bit 1 followed by k 0 bits
// 	then append the message with (uint64_t)msg_bit_length
void pad_msg(byte* msg, uint64_t msg_bit_length) {
	size_t pad_len = (msg_bit_length + 1) % MSG_LENGTH_MULTIPLE;
	size_t new_len;

	// allocate more padding
	// new_len is the new length
	if (pad_len > 448)
		new_len = (msg_bit_length + 1) + (512 - pad_len) + 512;
	else
		new_len = (msg_bit_length + 1) + (512 - pad_len);

	byte* tmp = realloc(msg, new_len);
	if (tmp == NULL) {
		printf("failed to pad message, memory allocation failed\n");
		free(msg);
		exit(2);
	}
	msg = tmp;

	size_t idx = msg_bit_length / 8; // byte index not bit index -- will be next byte if msg_bit_length % 8 == 0, => append 1 << 7
	msg[idx] |= 1 << (7 - (msg_bit_length % 8));

	// index of start of last 64 bits
	idx = (new_len - 64) / 8;
	// translate uint64_t to 8 bytes
	for (int i = 7; i >= 0; i--)
		msg[idx++] = (byte)((msg_bit_length >> (i * 8)) & (uint64_t)0xFF);
}

// SECTION: constants

// i'm sure there's a better way to do this
const word* const k_words() {
	if (g_k_words != NULL) return g_k_words;
	g_k_words = malloc(sizeof(word) * 4); // per stackoverflow post, OS should handle freeing of this
										  // since this is really just a singleton array, there's no runtime memory leak issue
	uint32_t_to_word_be(0x5a827999, &g_k_words[0]);
	uint32_t_to_word_be(0x6ed9eba1, &g_k_words[1]);
	uint32_t_to_word_be(0x8f1bbcdc, &g_k_words[2]);
	uint32_t_to_word_be(0xca62c1d6, &g_k_words[3]);
	return g_k_words;
}

const byte* k_t(short t) {
	if (t < 0 || t >= 80)
		throw_bad_t_val(t);

	const word* k = k_words();
	if (0 <= t && t <= 19)
		return k[0];
	else if (20 <= t && t <= 39)
		return k[1];
	else if (40 <= t && t <= 59)
		return k[2];
	else if (60 <= t && t <= 79)
		return k[3];
}

word* h_n(word* h, short n) {

	return h;
}

// initial hash value
const word* const h_0() {
	if (g_h_0 != NULL) return g_h_0;
	g_h_0 = malloc(sizeof(word) * 5);

	uint32_t_to_word_be(0x67452301, &g_h_0[0]);
	uint32_t_to_word_be(0xefcdab89, &g_h_0[1]);
	uint32_t_to_word_be(0x98badcfe, &g_h_0[2]);
	uint32_t_to_word_be(0x10325476, &g_h_0[3]);
	uint32_t_to_word_be(0xc3d2e1f0, &g_h_0[4]);
	return g_h_0;
}
