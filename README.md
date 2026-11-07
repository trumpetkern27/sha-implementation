# Secure Hashing Algorithm implementation

I wanted to implement each of the SHA functions to understand how they work. I felt this would be a good challenge and be fairly educational, providing me a deeper understanding of functionality that most take for granted. To aide the educational process, I decided to complete this without any AI--I am finding that struggling through failures and overcoming them is what truly crystalizes knowledge.

I figured also, while I'm at it, I may as well use this as an opportunity to learn other languages or deepen my current understanding.

Each SHA function will be delivered as an executable that takes 1 argument (the string wanting to be hashed) and print the hashed string in hex in the terminal. You may also redirect output into the executable (e.g., echo something | hash-function). Since this is just for educational purposes and won't be reused elsewhere, there is no reason to make anything more fancy than this.

## General note

Don't roll your own crypto. This is for educational purposes.

## MD4

This is implemented per Rivest's RFC 1320 memo (which replaced the original RFC 1186 document), written in JavaScript.

### Compiling

Who would ever want to compile JavaScript? Me, apparently.

```bash
cd md4
node --build-sea sea-config.json
```

### Things I learned

To start, it must be stated, JavaScript is so cursed. No native types?! I already knew that JavaScript was garbage, as I have some experience with TypeScript, but I didn't realize how abysmal it truly was. That being said, after writing in C, it is refreshing to not have to write as much boilerplate. However, it makes the code far less expressive, as I cannot strictly type things that I may want to, even if it has better out-of-the-box abstraction.

Here are the things I learned, both about the algorithm and JavaScript (respectively):

#### Endianness

I had already heard of endianness and understood basically what it was, but the MD4 documentation mixed endianness, which forced me to learn properly what it was. Bits stay in the same order within a byte, but bytes are organized either from least-to-most significant (little endian) or most-to-least significant (big endian).

While the concept itself is perhaps fairly trivial, once endianness is a concern in the code, it quickly melts your brain. In this implementation, this was a bug for a long time, as I did not realize the endianness was of any concern, but I finally discovered this and had to swap endianness at certain points for certain things.

#### There must be a more algebraic expression of this

While I did not spend the time to look into an algebraic expression for this or any of its kind, I noticed that many of the operations were effectively matrix transformations of some flavor. Additionally, the usage of constants like $\sqrt{3}$ (or at least the first 32 bits of them) also hinted towards this.

#### JavaScript doesn't have truly explicit typing

This sucks, there is a *very* good reason that TypeScript exists. Really, typing is limited to assignment and declarations. There is no way to force a parameter to be a specific type, but there are some types that exist that can be assigned to directly, and your LSP will yell at you if you mismatch them (e.g., mixing a BigInt with a number).

I understand that inferred typing makes development quicker, but it makes bugs *extremely* likely to occur, as there is hidden control flow, which then makes it rather unmaintainable.

While many modules do exist, and TypeScript also exists, it is unfortunate that typing is utterly opaque when developing in vanilla JavaScript. Of course, the *interpreter* knows what type something is, but it doesn't let you force types for variables, generally. Of course, there are certain points where this is possible, which I have taken great advantage of, but it is less strongly typed than even Python.

#### Strings are passed by value?!

I can't even...

#### ~ doesn't work well...

At this point, I can't remember explicitly what it was doing, but I do remember it was making some numbers negative when they shouldn't have been. I was using BigInts with a 32-bit mask, so it shouldn't have ever done this, but it did anyways, which ended up causing the addition down the road to not work. Perhaps I was not applying the mask at the correct point in time, but I found it exponentially easier to just write a bitwise not function and use that instead. Speed isn't the issue for this project, so who cares.

#### JavaScript is too framework-dependent

At first, I found deno was very easy to use to compile my code, and it just worked. However, once it got to the point to process something from stdin, it just wouldn't work. No matter what I did, it refused to acknowledge what I was trying to do with stdin, despite if I were to run something like `echo "a" | node src/main.js` it would work perfectly fine. 

So, I tried other things, and finally settled on just using node with its --build-sea flag. Unfortunately, the only way I was able to get this to work was to put *everything in to one file*. I thought this was utterly horrible, but I figured, it's JavaScript, it's not supposed to be a pleasant experience, why should I be able to write things in a way that makes sense?

Another issue with this: strings. The maximum length of a string is framework-dependent. I suppose in a sense, this makes sense, because any interpreter may have different rules. However, from a language standpoint, it makes no sense to have different standards for the same language.

#### JavaScript does weird things with argv

For some reason, JavaScript decides to have the first two arguments in argv as not null. To be specific, even *after compiling the code*, it ends up having `argv[1]` be the path to the file. Of course, if it were being run with node, this would make sense. But as a standalone binary?!

## SHA-1

This is implemented per FIPS PUB 180-1 and 180-4, written in C. This was tested against all test vectors in the appendix of FIPS PUB 180-1. Unfortunately, testing the $2^{33}$ bit length string which has been tested could not be verified on my machine, as it breaks a pipe.

### Compiling

```bash
cd sha
make
```

### Things I learned

Most things I learned were more related to C. I would like to understand the mathematics, but I will revisit this upon implementing MD4, MD5, and SHA-0, as it comes from that legacy. The only thing I really learned about SHA-1 was that those were its predecessors and that the NSA didn't release an explanation as to why it worked.

These are the things I learned and/or realized about C and some of the issues around those things.

#### `realloc` cannot be used for non-dynamically allocated memory

Linux started yelling at me about an "invalid pointer". This does make sense, though, since the memory for stack variables is managed by the OS -- otherwise it wouldn't know from the start where to free memory.

#### Pointers are stack variables too

I don't know why but I thought that if I passed a pointer into a function and changed it in there, it would change outside of that function as well, but I finally realized that if I want that behavior, I need to pass a pointer to that pointer and reassign the value that that pointer points to.

#### `calloc` is far superior to `malloc`

Testing on different operating systems yielded different results -- cause? garbage data I didn't assign.

This became clear when the `sha-1` function worked only for strings between 41-50 on Linux but then failed for those same strings on a different OS.

Of course, there is a place for malloc, e.g., if you fill *all* its data.

#### Do not use `strlen` after `malloc`

`malloc` can give you junk you don't want, even immediately after data you just copied there, which would increase the string length.

#### Reading piped output is freaking impossible

Once I got the function working for all test vectors I could type out, it came to the point where I wanted to test 1000000 "a" characters. That's too much for a terminal to really hold to put into an argv. *But*, I read that it could be done via piping output from one program to another.

After **much** struggle, I determined that this is possible by reading from `stdin`. I came across this answer rather early on in this journey. **However**. If I wanted to simply execute the program with *no* args, it would attempt to read from `stdin`. This was unacceptable -- I figured it should work like every other CLI where executing with no args shows its usage.

This took me down a rabbit hole of pipes. I still don't quite understand pipes, but I do understand that it was not what I wanted or needed. Half the posts on StackOverflow were recommending pipes to redirect `stdout` into the next program, the other half was just saying that it becomes `stdin`. I was stuck once I realized pipes were not the right answer.

At some point in this, I read that, in bash, piped commands are executed concurrently. Or at least, if you were to do something like `ls / | grep something` then grep starts at the same time as ls and waits for input. This got me thinking, perhaps I can watch for input from another thread. So that's what I did.

I feel like there absolutely has to be a better way to do this, but that's what I came up with and it works. Perhaps pipes were the right answer and I didn't dig deep enough. I don't know. But it works.
