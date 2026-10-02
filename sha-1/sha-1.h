typedef unsigned char byte;
typedef byte block[4][4]; // 4 x 4 matrix of bytes
typedef byte word[4]; // 4 bytes

void ch(word x, word y, word z, word* out);
void parity(word x, word y, word z, word* out);
void maj(word x, word y, word z, word* out);

void ft(short t, word x, word y, word z, word* out);

