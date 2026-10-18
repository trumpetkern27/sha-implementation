let uintArray = new Uint32Array([1, 2, 3, 4]);

uintArray = uintArray.map(
	s => (s << 1) | 1
)

console.log(uintArray);


let x = Number.MAX_SAFE_INTEGER;
console.log(x);
x *= 2;
console.log(x);
