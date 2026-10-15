#include "sha-1.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void throw_memcpy_error(void* p) {
	printf("failed to move memory to %p\n", p);
}

// SECTION: bitwise functions

word* rotl_n(size_t n, word x, word* out) {
	uint32_t x_32 = word_to_uint32_t_be(x); // convert to uint32_t to make life easier
	return uint32_t_to_word_be((x_32 << n) | (x_32 >> (WORD_BIT_LENGTH - n)), out);
}

word* rotr_n(size_t n, word x, word* out) {
	uint32_t x_32 = word_to_uint32_t_be(x);
	return uint32_t_to_word_be((x_32 >> n) | (x_32 << (WORD_BIT_LENGTH - n)), out);
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
uint64_t pad_msg(byte** msg, uint64_t msg_bit_length) {
	size_t pad_len = (msg_bit_length + 1) % MSG_LENGTH_MULTIPLE;
	size_t new_len;

	// allocate more padding
	// new_len is the new length in bytes
	if (pad_len > 448)
		new_len = ((msg_bit_length + 1) + (512 - pad_len) + 512) / 8;
	else
		new_len = ((msg_bit_length + 1) + (512 - pad_len)) / 8;

	byte* tmp = calloc(new_len, sizeof(byte)); // need to have clear memory
	if (tmp == NULL) {
		printf("failed to pad message, memory allocation failed\n");
		free(*msg);
		exit(2);
	}
	memcpy(tmp, *msg, msg_bit_length);
	*msg = tmp;

	size_t idx = msg_bit_length / 8; // byte index not bit index -- will be next byte if msg_bit_length % 8 == 0, => append 1 << 7
	(*msg)[idx] |= 1 << (7 - (msg_bit_length % 8));

	// index of start of last 64 bits
	idx = new_len - 8;
	// grab each byte and move to memory
	for (size_t i = 8; i > 0; i--)
		(*msg)[new_len - i] = (msg_bit_length >> ((i - 1) * 8)) & 0xFF;

	return new_len;
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

byte* k_t(short t) { // i hate c
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

// SECTION: schedule

// the tth message word is defined as the ith message block for 0 <= t <= 15 or
// ROTL^1(W_{t-3} ^ W_{t-8} ^ W_{t-14} ^ W_{t-16}) for 16 <= t <= 79
word* w_t(word* m, short t, word* out) {
	if (t < 0 || t >= 80)
		throw_bad_t_val(t);

	if (0 <= t && t <= 15)
		return &m[t];
	else {
		uint32_t w_3 = word_to_uint32_t_be(*w_t(m, t - 3, out));
		uint32_t w_8 = word_to_uint32_t_be(*w_t(m, t - 8, out));
		uint32_t w_14 = word_to_uint32_t_be(*w_t(m, t - 14, out));
		uint32_t w_16 = word_to_uint32_t_be(*w_t(m, t - 16, out));
		word ret;
		return rotl_n(1, *uint32_t_to_word_be(w_3 ^ w_8 ^ w_14 ^ w_16, &ret), out);
	}
}

// SECTION: main algo

// returns a string of the output in hex
char* sha1(byte* msg, char* output, size_t len) {
	size_t blen = strlen((char*)msg) * 8; // bit length

	size_t new_byte_len = (size_t)pad_msg(&msg, blen); // padding message returns new byte length -- make word* flavor for ease of access
	word* message = (word*)&msg;
	if (message == NULL) {
		printf("failed to copy message, memory allocation failed\n");
		free(msg);
		exit(2);
	}

	size_t n = new_byte_len / MSG_BYTE_LENGTH; // message array size

	word h0_i, h1_i, h2_i, h3_i, h4_i;
	// we can just call h_0() and not cache since it's already cached by g_h_0
	memcpy(&h0_i, h_0()[0], sizeof(word));
	memcpy(&h1_i, h_0()[1], sizeof(word));
	memcpy(&h2_i, h_0()[2], sizeof(word));
	memcpy(&h3_i, h_0()[3], sizeof(word));
	memcpy(&h4_i, h_0()[4], sizeof(word));
	for (size_t i = 0; i < n; i++) { // docs say "1 to N" but who wants to use 1-based indexing?
		word a, b, c, d, e, T; // using the same names as the docs
		word tmp;

		memcpy(&a, &h0_i, sizeof(word));
		memcpy(&b, &h1_i, sizeof(word));
		memcpy(&c, &h2_i, sizeof(word));
		memcpy(&d, &h3_i, sizeof(word));
		memcpy(&e, &h4_i, sizeof(word));

		for (short t = 0; t < 80; t++) {
			// luckily C computes overflow for unsigned as mod n
			uint32_t_to_word_be( // we'll just ignore the return value because it's just &T
								 // and it would be way too easy if i just assign &T to itself
				word_to_uint32_t_be(*rotl_n(5, a, &tmp))
				+ word_to_uint32_t_be(*f_t(t, b, c, d, &tmp))
				+ word_to_uint32_t_be(e)
				+ word_to_uint32_t_be(k_t(t))
				+ word_to_uint32_t_be(*w_t(&message[i], t, &tmp)),
				&T // <- return
			);

			memcpy(&e, &d, sizeof(word));
			memcpy(&d, &c, sizeof(word));
			rotl_n(30, b, &c); // same story as &T
			memcpy(&b, &a, sizeof(word));
			memcpy(&a, &T, sizeof(word));
		}

		// compute the ith intermediate value for H^i
		uint32_t_to_word_be(
			word_to_uint32_t_be(a) + word_to_uint32_t_be(h0_i),
			&h0_i
		);
		uint32_t_to_word_be(
			word_to_uint32_t_be(b) + word_to_uint32_t_be(h1_i),
			&h1_i
		);
		uint32_t_to_word_be(
			word_to_uint32_t_be(c) + word_to_uint32_t_be(h2_i),
			&h2_i
		);
		uint32_t_to_word_be(
			word_to_uint32_t_be(d) + word_to_uint32_t_be(h3_i),
			&h3_i
		);
		uint32_t_to_word_be(
			word_to_uint32_t_be(e) + word_to_uint32_t_be(h4_i),
			&h4_i
		);
	}

	char* cur = output;
	char* const end = output + len;
	cur += snprintf(cur, end - cur, "0x");
	for (size_t i = 0; i < 4; i++)
		cur += snprintf(cur, end - cur, "%02X", h0_i[i]);
	for (size_t i = 0; i < 4; i++)
		cur += snprintf(cur, end - cur, "%02X", h1_i[i]);
	for (size_t i = 0; i < 4; i++)
		cur += snprintf(cur, end - cur, "%02X", h2_i[i]);
	for (size_t i = 0; i < 4; i++)
		cur += snprintf(cur, end - cur, "%02X", h3_i[i]);
	for (size_t i = 0; i < 4; i++)
		cur += snprintf(cur, end - cur, "%02X", h4_i[i]);

	snprintf(cur, end - cur, "\n");
	
	free(message);

	// for pretty output
	return output;
}
