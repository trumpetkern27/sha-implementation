import { md4 } from "./md4.js"
let uintArray = new Uint32Array([1, 2, 3, 4]);

uintArray = uintArray.map(
	s => (s << 1) | 1
)

console.log(uintArray);
