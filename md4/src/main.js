import { md4 } from "./md4.js"

let s = "";
if (process.stdin.isTTY !== undefined) {
	console.log(process.stdin);
} else if (process.argv.length === 3) {
	s = process.argv[2];
} else {
	console.log("Usage: [node] md4 <input>")
	process.exit(1);
}

console.log(md4(s));
