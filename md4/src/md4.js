// MD4 algorithm

// SECTION: helpers

const WORD_BIT_LENGTH = 32n;
const MASK = 0xFFFFFFFFn;

// left rotates a 32bit number by n
function rotl_n(num, n) {
	console.log("		num: " + num.toString(2) + " n: " + n);
	return ((BigInt(num) << BigInt(n)) | ((BigInt(num) & MASK) >> (WORD_BIT_LENGTH - BigInt(n)))) & MASK
}

// bc freaking javascript
function not(x) {
	let X = BigInt(x);
	let ret = 0n;
	for (let i = 0n; i < 32n; i++)
		ret |= (1n - ((X >> i) & 1n)) << i;
	return ret;
}

// SECTION: message handling

function pad_msg(msg) {
	/* i hate javascript
     * just when you think it couldn't get any worse...
     * they decide that passing strings is by value not by reference
	 */
	let out = msg.split('');
	out.push(String.fromCharCode(0b10000000)); // pad with 1 bit

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
	byte_len--; // bc 1 bit was padded, it is off
	// javascript decides to make all numbers as IEEE 754 doubles, so it needs to be converted
	// BUT it will not let me use the double_to_uint64 function here

	let message_bit_len = byte_len * 8n;

	// need to add 1 to the length bc of padding bit -- since it will always be text, it is fine to add 1 to the byte_len rather than bit length
	let new_len = byte_len + 1n;
	const mod = (((new_len * 8n) % 512n) + 512n) % 512n // make sure it's positive
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
	console.log("			(f)")
	console.log("				 x: " + x.toString(2));
	console.log("				 y: " + y.toString(2));
	console.log("				~x: " + (not(x)).toString(2));
	console.log("				 z: " + z.toString(2));
	console.log("			(f) = " + (((BigInt(x) & BigInt(y)) | (not(x) & BigInt(z))) & MASK).toString(2));
	return ((BigInt(x) & BigInt(y)) | (not(x) & BigInt(z))) & MASK;
}
function g(x, y, z) {
	console.log("			(g)")
	console.log("				 x: " + x.toString(2));
	console.log("				 y: " + y.toString(2));
	console.log("				 z: " + z.toString(2));
	console.log("			(g) = " + (((BigInt(x) & BigInt(y)) | (BigInt(x) & BigInt(z)) | (BigInt(y) & BigInt(z))) & MASK).toString(2))
	return ((BigInt(x) & BigInt(y)) | (BigInt(x) & BigInt(z)) | (BigInt(y) & BigInt(z))) & MASK;
}
function h(x, y, z) {
	console.log("			(h)")
	console.log("				 x: " + x.toString(2));
	console.log("				 y: " + y.toString(2));
	console.log("				 z: " + z.toString(2));
	console.log("			(h) = " + ((BigInt(x) ^ BigInt(y) ^ BigInt(z)) & MASK).toString(2));
	return (BigInt(x) ^ BigInt(y) ^ BigInt(z)) & MASK;
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

	for (let i = 0; i < Number(len / 64n); i++) {
		// copy block into buffer
		for (let j = 0; j < 16; j++) { // operate on 32 bit words
			X[j] = 0n;
			for (let k = 0n; k < 4n; k++)
				X[j] = X[j] | (BigInt(res.message[i * 64 + j * 4 + Number(k)].charCodeAt(0)) << ((3n - k) * 8n)) & MASK;
		}

		// same flavor as RFC 1320
		let AA = A;
		let BB = B;
		let CC = C;
		let DD = D;

		// round 1
		for (let j = 0; j < 4; j++) {
			console.log("j: " + j);
			console.log("	A: " + A.toString(2) + " X" + X[0 + j * 4].toString(2));
			A = rotl_n((A + f(B, C, D) + X[0 + j * 4]) & MASK, 3n);
			console.log("	A: " + A.toString(2));

			console.log("	D: " + D.toString(2) + " X" + X[1 + j * 4].toString(2));
			D = rotl_n((D + f(A, B, C) + X[1 + j * 4]) & MASK, 7n);
			console.log("	D: " + D.toString(2));

			console.log("	C: " + C.toString(2) + " X" + X[2 + j * 4].toString(2));
			C = rotl_n((C + f(D, A, B) + X[2 + j * 4]) & MASK, 11n);
			console.log("	C: " + C.toString(2));

			console.log("	B: " + B.toString(2) + " X" + X[3 + j * 4].toString(2));
			B = rotl_n((B + f(C, D, A) + X[3 + j * 4]) & MASK, 19n);
			console.log("	B: " + B.toString(2));
		}
		console.log("A: " + A.toString(16));
		console.log("D: " + D.toString(16));
		console.log("C: " + C.toString(16));
		console.log("B: " + B.toString(16));

		// round 2
		for (let j = 0; j < 4; j++) {
			A = rotl_n((A + g(B, C, D) + X[j + 0] + 0x5A827999n) & MASK, 3n);
			D = rotl_n((D + g(A, B, C) + X[j + 4] + 0x5A827999n) & MASK, 5n);
			C = rotl_n((C + g(D, A, B) + X[j + 8] + 0x5A827999n) & MASK, 9n);
			B = rotl_n((B + g(C, D, A) + X[j + 12] + 0x5A827999n) & MASK, 13n);
		}

		// round 3
		A = rotl_n((A + h(B, C, D) + X[0] + 0x6ED9EBA1n) & MASK, 3n);
		D = rotl_n((D + h(A, B, C) + X[8] + 0x6ED9EBA1n) & MASK, 9n);
		C = rotl_n((C + h(D, A, B) + X[4] + 0x6ED9EBA1n) & MASK, 11n);
		B = rotl_n((B + h(C, D, A) + X[12] + 0x6ED9EBA1n) & MASK, 15n);
		A = rotl_n((A + h(B, C, D) + X[2] + 0x6ED9EBA1n) & MASK, 3n);
		D = rotl_n((D + h(A, B, C) + X[10] + 0x6ED9EBA1n) & MASK, 9n);
		C = rotl_n((C + h(D, A, B) + X[6] + 0x6ED9EBA1n) & MASK, 11n);
		B = rotl_n((B + h(C, D, A) + X[14] + 0x6ED9EBA1n) & MASK, 15n);
		A = rotl_n((A + h(B, C, D) + X[1] + 0x6ED9EBA1n) & MASK, 3n);
		D = rotl_n((D + h(A, B, C) + X[9] + 0x6ED9EBA1n) & MASK, 9n);
		C = rotl_n((C + h(D, A, B) + X[5] + 0x6ED9EBA1n) & MASK, 11n);
		B = rotl_n((B + h(C, D, A) + X[13] + 0x6ED9EBA1n) & MASK, 15n);
		A = rotl_n((A + h(B, C, D) + X[3] + 0x6ED9EBA1n) & MASK, 3n);
		D = rotl_n((D + h(A, B, C) + X[11] + 0x6ED9EBA1n) & MASK, 9n);
		C = rotl_n((C + h(D, A, B) + X[7] + 0x6ED9EBA1n) & MASK, 11n);
		B = rotl_n((B + h(C, D, A) + X[15] + 0x6ED9EBA1n) & MASK, 15n);

		A = (A + AA) & MASK;
		B = (B + BB) & MASK;
		C = (C + CC) & MASK;
		D = (D + DD) & MASK;
	}

	return A.toString(16) + ' ' + B.toString(16) + ' ' + C.toString(16) + ' ' + D.toString(16);
}
