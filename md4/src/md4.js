// MD4 algorithm

// SECTION: helpers

const WORD_BIT_LENGTH = 32;

function rotl_n(num, n) {
	return (num << n) | (num >>> (WORD_BIT_LENGTH - n))
}
function rotr_n(num, n) {
	return (num >>> n) | (num << (WORD_BIT_LENGTH - n))
}

function double_to_uint64(d) {
	let b = BigInt(d);
	b = ((b >> 52n) & 0b011111111111n) - 1023n;  // 11 bits + leading 0 for sign
	return b
}

// SECTION: message handling

function pad_msg(msg) {
	/* i hate javascript
     * just when you think it couldn't get any worse...
     * they decide that passing strings is by value not by reference
	 */
	let out = msg.split('');
	out.push(String.fromCharCode(0b1000000)); // pad with 1 bit

	// FIXME: actually javascript only supports strings up to length 2^{53} - 1
	// this needs to be longer

	// javascript *might* not support strings of a certain length
	// IF by some chance it does support a string of a length longer then 2^{53} - 1,
	// 	it needs to be fixed bc the .length() will return an int64
	let byte_len = BigInt(0);
	try {
		byte_len = BigInt(out.length);
	} catch {
		byte_len = BigInt(Number.MAX_SAFE_INTEGER);
		let tmp = 0n;
		while (out[Number(byte_len + tmp++)] !== '\0') { }
		byte_len += tmp - 1n;
	}
	// javascript decides to make all numbers as IEEE 754 doubles, so it needs to be converted
	// BUT it will not let me use the double_to_uint64 function here

	let message_bit_len = byte_len * 8n;

	let new_len = byte_len;
	const mod = (((byte_len * 8n) % 512n) + 512n) % 512n // make sure it's positive
	// pad with 0 bits until last 512 bit block has room for a full uint64 in the last 64 bits
	// just let the last two bytes be 0x00, will fill in later
	const nullstring = String.fromCharCode(0); // in case javascript wants to be weird
	if (mod <= 448) { // ensure modulus always positive
		for (let i = mod / 8n; i < 512n / 8n; i++) {
			out.push(nullstring);
			new_len++;
		}
	} else {
		for (let i = mod / 8n; i < 2n * 512n / 8n; i++) {
			out.push(nullstring);
			new_len++;
		}
	}

	// so
	// in contrast to SHA-1, MD4 does not technically have a message length cap
	// BUT
	// it does only record the last 64 bits of the original message length

	for (let i = BigInt(8); i > 0; i--)
		out[Number(new_len - i)] = String
			.fromCharCode(
				Number(
					message_bit_len >> ((i - 1n) * 8n) & 0xFFn
				)
			);

	return {message: out, new_len};
}

// SECTION: functions for hash

function f(x, y, z) {
	return (BigInt(x) & BigInt(y)) | (~BigInt(x) & BigInt(z))
}
function g(x, y, z) {
	return (BigInt(x) & BigInt(y)) | (BigInt(x) & BigInt(z)) | (BigInt(y) & BigInt(z))
}
function h(x, y, z) {
	return BigInt(x) ^ BigInt(y) ^ BigInt(z)
}

export function md4(m) {
	const res = pad_msg(m);
	const len = res.new_len;

	let X = Array(16);
	// loop through each 16 word (512 bit) block
	let A = 0x01234567n;
	let B = 0x89abcdefn;
	let C = 0xfedcba98n;
	let D = 0x76543210n;

	for (let i = 0; i < len / 64n; i++) {
		// copy block into buffer
		for (let j = 0; j < 16; j++) // operate on 32 bit words
			for (let k = 0n; k < 4n; k++)
				X[j] = BigInt(res.message[i * 64 + j * 4 + Number(k)].charCodeAt(0)) << (3n - k);

		// same flavor as RFC 1320
		let AA = A;
		let BB = B;
		let CC = C;
		let DD = D;

		// round 1
		for (let j = 0; j < 4; j++) {
			A = (A + f(B, C, D) + X[0 + j * 4]) << 3n;
			D = (D + f(A, B, C) + X[1 + j * 4]) << 7n;
			C = (C + f(D, A, B) + X[2 + j * 4]) << 11n;
			B = (B + f(A, B, C) + X[3 + j * 4]) << 19n;
		}

		// round 2
		for (let j = 0; j < 4; j++) {
			A = (A + g(B, C, D) + X[0 + j * 4] + 0x5A827999n) << 3n;
			B = (B + g(C, D, A) + X[1 + j * 4] + 0x5A827999n) << 5n;
			C = (C + g(D, A, B) + X[2 + j * 4] + 0x5A827999n) << 9n;
			D = (D + g(A, B, C) + X[3 + j * 4] + 0x5A827999n) << 13n;
		}

		// round 3
		for (let j = 0; j < 4; j++) {
			A = (A + h(B, C, D) + X[0 + j * 4] + 0x6ED9EBA1n) << 3n;
			B = (B + h(C, D, A) + X[1 + j * 4] + 0x6ED9EBA1n) << 9n;
			C = (C + h(D, A, B) + X[2 + j * 4] + 0x6ED9EBA1n) << 11n;
			D = (D + h(A, B, C) + X[3 + j * 4] + 0x6ED9EBA1n) << 15n;
		}

		A += AA;
		B += BB;
		C += CC;
		D += DD;
	}

	return A.toString(16) + ' ' + B.toString(16) + ' ' + C.toString(16) + ' ' + D.toString(16);
}
