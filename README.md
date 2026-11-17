# Simple Encryption Using XOR

COMP 211 Checkoff 3: Applications of Bitwise Operators, Simple Encryption Using XOR
Xiangyi Gao

## What I explored

I wanted to understand how the `^` operator we learned in COMP 211 can be used to encrypt data, and why simple XOR encryption is not secure. I learned it by talking with an AI tutor (Claude), writing and running C programs, and checking the AI's claims against my own output.

The path I took:

1. Why XOR undoes itself: `(a ^ k) ^ k == a`
2. Encrypting a string with a single-byte key
3. Breaking a single-byte key by trying all 256 keys
4. Breaking a repeating key like `"SECRET"` by splitting the ciphertext into columns
5. Why a one-time pad cannot be broken
6. How real ciphers (AES, ChaCha20) still use XOR, but safely

## Most interesting discoveries

**1. The same code encrypts and decrypts.** XOR flips the bits where the key has a 1. Doing it again flips them back.

```
  01001000   'H'
^ 00101010   key
----------
  01100010   encrypted
^ 00101010   same key
----------
  01001000   'H' again
```

**2. A wrong key can give an almost-right answer.** When I printed the score for every key, key 109 decrypted the message to `meet.me.at.the.library.at.noon`: the right message in lowercase. The reason is that `109 = 77 ^ 32`, and 32 (bit 5) is exactly the bit that separates uppercase and lowercase letters in ASCII. Spaces became byte 0, which is why it only scored 24/30. (The AI first said 25. My output showed 24, and it agreed it had miscounted.)

**3. A longer repeating key does not help much.** A 6-byte key sounds like 256⁶ possibilities, but each key byte can be broken separately, so it only takes 6 × 256 tries. The correct key length stands out because its score jumps:

```
keylen 3: score 159/171  key = SEC
keylen 6: score 169/171  key = SECRET   <- found it
keylen 7: score 146/171  key = SREEERT
```

When I first ran this, I changed the key to `"SECRET"` but forgot to update `secretlen`, so the program only found `"SEC"`. The attack was right: it found the key that was really used.

**4. Ciphertext is not a string.** In my example, `'M'` is 77 in ASCII and the key is also 77, so `'M' ^ 77 = 0`. The ciphertext starts with a `'\0'`, so `strlen` returns 0 and `printf("%s")` prints nothing. Encrypted data must be handled with a saved length, not with string functions.

## Summary in my own words

XOR undoes itself, so the same operation encrypts and decrypts. XOR is not the weak part; a short, repeating key is. Because English text has patterns, an attacker can split the ciphertext into columns, try all 256 values for each key byte, and try different key lengths until one scores almost perfectly, as long as the message is long enough. Some wrong keys give similar-looking output because of how the ASCII table is laid out.

A one-time pad (a random key as long as the message, never reused) is safe because every possible message has some key that produces the ciphertext, so the attacker learns nothing. AES and ChaCha20 still finish with `ciphertext = plaintext ^ keystream`, but they mix XOR with other operations over many rounds to make a keystream that never repeats and cannot be predicted. No practical attack on their full versions has been found.

XOR is used to combine the key with the data because it scrambles the bits without losing information, so the key can undo it. `&` and `|` lose information, so they are used for other jobs, like masking bytes and building rotations.

**Key takeaway:** XOR is a good way to combine data with a key. The security depends entirely on the keystream: it must be unpredictable and never repeated.

## Running the code

The programs are in [`code/`](code). You need `gcc` (works in the COMP 211 `learncli` container).

```bash
cd code
make                      # builds all four programs
./1_xor_basic             # encrypt and decrypt "HELLO"
./2_break_single_byte     # break a single-byte key, show close-scoring keys
./3_break_repeating_key   # break the repeating key "SECRET"
./4_one_time_pad          # same ciphertext -> "ATTACK" or "DEFEND"
make clean
```

Or compile one file by itself, for example `gcc -Wall -o 1_xor_basic 1_xor_basic.c`.

| File | What it shows |
|---|---|
| `1_xor_basic.c` | Same loop encrypts and decrypts |
| `2_break_single_byte.c` | Brute-force all 256 keys with a letter/space score; the `'\0'` in the ciphertext; key 109 |
| `3_break_repeating_key.c` | Split into columns, try key lengths 1–8 |
| `4_one_time_pad.c` | Why a one-time pad leaks nothing |

## Presentation

Slides: [`slides/xor-encryption-slides.pdf`](slides/xor-encryption-slides.pdf)

## AI conversation

- Full conversation: https://claude.ai/share/243af0bc-f46f-44f9-9967-91f8534d3168
- An earlier, shorter exploration in Claude's desktop app (no share link available there): [`ai/earlier-conversation.md`](ai/earlier-conversation.md)

## Resources

- [XOR cipher (Wikipedia)](https://en.wikipedia.org/wiki/XOR_cipher)
- [One-time pad (Wikipedia)](https://en.wikipedia.org/wiki/One-time_pad)
- [Cryptopals Crypto Challenges, Set 1](https://cryptopals.com/sets/1): challenges 3 and 6 are the single-byte and repeating-key XOR attacks
- [FIPS 197: the AES standard](https://csrc.nist.gov/pubs/fips/197/final)
- [RFC 8439: ChaCha20 and Poly1305](https://www.rfc-editor.org/rfc/rfc8439)
- [ASCII table](https://www.asciitable.com/)
