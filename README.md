# Secure Hashing Algorithm implementation

I wanted to implement each of the SHA functions to understand how they work. I felt this would be a good challenge and be fairly educational, providing me a deeper understanding of functionality that most take for granted. To aide the educational process, I decided to complete this without any AI--I am finding that struggling through failures and overcoming them is what truly crystalizes knowledge.

I figured also, while I'm at it, I may as well use this as an opportunity to learn other languages or deepen my current understanding.

Each SHA function will be delivered as an executable that takes 1 argument (the string wanting to be hashed) and print the hashed string in hex in the terminal. Since this is just for educational purposes and won't be reused elsewhere, there is no reason to make anything more fancy than this.

## General note

Don't roll your own crypto. This is for educational purposes.

## SHA-1

This is implemented per FIPS PUB 180-1 and 180-4, written in C. This was tested against all test vectors in the appendix of FIPS PUB 180-1. Unfortunately, testing the $2^{33}$ bit length string which has been tested could not be verified on my machine, as it breaks a pipe.

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
