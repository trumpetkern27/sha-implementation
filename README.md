# Secure Hashing Algorithm implementation

I wanted to implement each of the SHA functions to understand how they work. I felt this would be a good challenge and be fairly educational, providing me a deeper understanding of functionality that most take for granted. To aide the educational process, I decided to complete this without any AI--I am finding that struggling through failures and overcoming them is what truly crystalizes knowledge.

I figured also, while I'm at it, I may as well use this as an opportunity to learn other languages or deepen my current understanding.

Each SHA function will be delivered as an executable that takes 1 argument (the string wanting to be hashed) and print the hashed string in hex in the terminal. Since this is just for educational purposes and won't be reused elsewhere, there is no reason to make anything more fancy than this.

## General note

Don't roll your own crypto. This is for educational purposes.

## SHA-1

This is implemented per FIPS NIST 180-4, written in C.

### Things I learned

- `realloc` cannot be used for non-dynamically allocated memory (i.e., pointers to stack variables cannot be used with `realloc`)
