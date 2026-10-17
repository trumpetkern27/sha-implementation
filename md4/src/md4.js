// MD4 algorithm

// SECTION: helpers

const WORD_BIT_LENGTH = 32;

function rotl_n(num, n) {
	return (num << n) | (num >>> (WORD_BIT_LENGTH - n))
}
function rotr_n(num, n) {
	return (num >>> n) | (num << (WORD_BIT_LENGTH - n))
}

// SECTION: message handling

function pad_msg(msg) {
	let out = msg;
	const byte_len = msg.length();

	out.append(0b1000000);
	if (((byte_len - 1) * 8) % 512 <= 448) {
		for (let i = 0; i < )
	} else {

	}



	return out;
}

