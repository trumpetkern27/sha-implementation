// SECTION: md4

// so...
// in order to make a working binary, everything needs to be in one file
// because javascript

// SECTION: helpers

const WORD_BIT_LENGTH = 32n;
const MASK = 0xFFFFFFFFn;

// left rotates a 32bit number by n
function rotl_n(num, n) {
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

// swaps endianness for a 32 bit variable
function swap_endian_32(x) {
	let X = BigInt(x);
	return 0n
		| ((X & 0xFFn) << 24n)
		| ((X & 0xFF00n) << 8n)
		| ((X & 0xFF0000n) >> 8n)
		| ((X & 0xFF000000n) >> 24n);
}

// SECTION: message handling

function pad_msg(msg) {
	/* i hate javascript
     * just when you think it couldn't get any worse...
     * they decide that passing strings is by value not by reference
	 */
	let out = msg.split('');
	out.push(String.fromCharCode(0b10000000)); // pad with 1 bit

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

	// need to add 1 to the length bc of padding bit -- since it will always be text, it is fine to add 1 to the byte_len
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
	// TECHNICALLY, the md4 function does not have a maximum message length
	// per the documentation, only the lower 64 bits are used of the length if it exceeds 2^64

	// as per the documentation (section 3.2), this is appended in 32-bit words in lower order
	// so the order in which each half of the uint64 is appended is reversed from what would be expected
	const loword = (byte_len << 3n) & MASK;
	const hiword = (byte_len >> 29n) & MASK;
	for (let i = BigInt(8); i > 0; i--) {
		out[Number(new_len - i)] = String
			.fromCharCode(
				Number(
					(i > 4
						? loword
						: hiword
					) >> ((i % 4n - 1n) * 8n) & 0xFFn
				)
			)
	}
	return {message: out, new_len};
}

// SECTION: functions for hash

function f(x, y, z) {
	return ((BigInt(x) & BigInt(y)) | (not(x) & BigInt(z))) & MASK;
}
function g(x, y, z) {
	return ((BigInt(x) & BigInt(y)) | (BigInt(x) & BigInt(z)) | (BigInt(y) & BigInt(z))) & MASK;
}
function h(x, y, z) {
	return (BigInt(x) ^ BigInt(y) ^ BigInt(z)) & MASK;
}

// SECTION: hash

function md4(m) {
	const res = pad_msg(m);
	const len = res.new_len;

	let X = Array(16);
	// basically everything is expected to be inputted in little endian, so endianness needs to be swapped
	// ... basically ...
	let A = swap_endian_32(0x01234567n);
	let B = swap_endian_32(0x89abcdefn);
	let C = swap_endian_32(0xfedcba98n);
	let D = swap_endian_32(0x76543210n);

	// loop through each 16 word (512 bit) block
	for (let i = 0; i < Number(len / 64n); i++) {
		// copy block into buffer
		for (let j = 0; j < 16; j++) { // operate on 32 bit words
			X[j] = 0n;
			for (let k = 0n; k < 4n; k++) {
				// here the endianness of the length appended to the message is not translated to little endian
				// it was assumed that everything else was put into the message in little endian
				// but technically the "64-bit representation" of the length is appended to the end for padding
				// which makes it still in big endian
				X[j] = i === Number(len / 64n) - 1 && j >= 14
					? X[j] | (BigInt(res.message[i * 64 + j * 4 + Number(k)].charCodeAt(0)) << ((3n - k) * 8n)) & MASK
					: X[j] | swap_endian_32((BigInt(res.message[i * 64 + j * 4 + Number(k)].charCodeAt(0)) << ((3n - k) * 8n)) & MASK);
			}
		}

		// same flavor as RFC 1320
		let AA = A;
		let BB = B;
		let CC = C;
		let DD = D;

		// round 1 -- loop to make it more visually pleasing
		for (let j = 0; j < 4; j++) {
			A = rotl_n((A + f(B, C, D) + X[0 + j * 4]) & MASK, 3n);
			D = rotl_n((D + f(A, B, C) + X[1 + j * 4]) & MASK, 7n);
			C = rotl_n((C + f(D, A, B) + X[2 + j * 4]) & MASK, 11n);
			B = rotl_n((B + f(C, D, A) + X[3 + j * 4]) & MASK, 19n);
		}

		// round 2
		// in constrast to most everything else, the constants here are provided in big endian per the docs
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

	return swap_endian_32(A).toString(16).padStart(8, '0')
		+ swap_endian_32(B).toString(16).padStart(8, '0')
		+ swap_endian_32(C).toString(16).padStart(8, '0')
		+ swap_endian_32(D).toString(16).padStart(8, '0');
}

// SECTION: main
// prints md4 hash
function output(input) {
	console.log(md4(input));
}

// get stdin if passed in
var stdin = process.stdin;
if (stdin.isTTY === undefined) {
	let s = "";
	stdin.on('readable', function() {
		var chunk = this.read(); // my LSP is crying...
		if (chunk !== null)
			s += chunk;
	});
	stdin.on('end', function() {
		output(s.substring(0, s.length - 1));
	});
} else if (process.argv.length === 3) { // because javascript has argv[1] as the path to the file...
	output(process.argv[2]);
} else {
	console.log("Usage: [node] md4 <input>")
	process.exit(1);
}
