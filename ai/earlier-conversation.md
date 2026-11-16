# AI Conversation: XOR Encryption Basics and Security

Tool: Claude (Sonnet 5.5), Claude desktop app. Exported 2026-11-15.

## Me

I’m a student in COMP 211 (systems programming in C). I know C, pointers, and bitwise operators like &, |, ^. I want to learn how XOR can be used for simple encryption and why it’s not secure. Can you explain the basic idea at my level?

## Claude

# XOR as a cipher

## The key property

XOR is its own inverse:

```
a ^ b ^ b == a
```

Any bit XORed with the same bit twice returns to its original value. So if `k` is a secret key, then:

- **Encrypt:** `cipher = plain ^ k`
- **Decrypt:** `plain = cipher ^ k`

The same operation does both jobs.

## In C

```c
#include <stdio.h>
#include <string.h>

// Same function encrypts and decrypts.
void xor_crypt(char *buf, size_t n, const char *key, size_t klen) {
    for (size_t i = 0; i < n; i++)
        buf[i] ^= key[i % klen];   // key repeats if shorter than the message
}

int main(void) {
    char msg[] = "attack at dawn";
    const char *key = "k3y";

    xor_crypt(msg, strlen(msg), key, strlen(key));  // now gibberish
    xor_crypt(msg, strlen(msg), key, strlen(key));  // back to the original
    printf("%s\n", msg);
}
```

## Why it's not secure

**1. Known plaintext reveals the key.** XOR also works backwards: if `c = p ^ k`, then `k = c ^ p`. If an attacker knows or guesses even part of the message, they recover that part of the key. File headers are a good example. A PNG always starts with the same 8 bytes, and a text file may start with `"From:"`. XORing those known bytes with the ciphertext leaks the matching key bytes.

**2. A short repeating key leaves patterns.** With a 3-byte key, every 3rd byte uses the same key byte. The attacker can split the ciphertext into 3 groups and treat each as a single-byte XOR. Each group falls to frequency analysis. For example, the most common byte in English text is usually a space (`0x20`), so the most common ciphertext byte in a group is probably `space ^ key_byte`.

**3. Reusing a key leaks the plaintexts.** If two messages use the same key, then `c1 ^ c2 = (p1 ^ k) ^ (p2 ^ k) = p1 ^ p2`. The key cancels out. What's left is the two plaintexts XORed together, which is often easy to untangle.

**4. No integrity.** An attacker who knows where a field sits in the message can flip bits in the ciphertext. Those same bits flip in the plaintext after decryption, and you get no warning. XORing the ciphertext with `"100" ^ "900"` turns a "100" in the plaintext into "900".

## The one case where it is secure

The **one-time pad** uses a key that is truly random, as long as the message, and never reused. It is provably unbreakable. The catch is that you need to share a key as big as your data, which is rarely practical. The problems above all come from relaxing one of those three conditions.

Real ciphers like AES-CTR and ChaCha20 are built from the same idea. They generate a long pseudorandom keystream from a short key and XOR it with the data. The security comes from how the keystream is generated, not from the XOR itself.

## Try it

Write a program that breaks a single-byte XOR key by trying all 256 values and scoring each result by how many bytes are letters or spaces. It shows how weak the scheme is, and it's a classic exercise (Cryptopals set 1).

I haven't compiled the C example above, and it's for learning only. Don't use XOR ciphers for anything real.

## Me

Why is XOR its own inverse? Can you explain the equation a ^ b ^ b == a to me? The program you gave is a little bit complicated for me. Can you show me a shorter C program that encrypts and decrypts a string using XOR with a key?

## Claude

## Why `a ^ b ^ b == a`

XOR works on one bit at a time. Its rule is: **the result is 1 if the two bits differ, 0 if they're the same.**

| x | y | x ^ y |
|---|---|-------|
| 0 | 0 | 0 |
| 0 | 1 | 1 |
| 1 | 0 | 1 |
| 1 | 1 | 0 |

Two facts follow from the table:

1. **`x ^ x == 0`**: a bit XORed with itself always gives 0, because the bits are the same.
2. **`x ^ 0 == x`**: XORing with 0 changes nothing. Look at the rows where y is 0.

XOR is also associative, so `a ^ b ^ b` can be regrouped as `a ^ (b ^ b)`:

```
a ^ b ^ b
= a ^ (b ^ b)    // regroup
= a ^ 0          // fact 1
= a              // fact 2
```

Here is one bit at a time with `a = 1`, `b = 1`:

```
1 ^ 1 = 0    (a ^ b: the bit flipped)
0 ^ 1 = 1    (^ b again: it flipped back)
```

XORing with a 1 bit **flips** a bit, and XORing with a 0 bit leaves it alone. Applying the same flips twice puts everything back. This works for every bit of every byte, so it works on whole bytes and whole values.

## A shorter program

This uses a single-character key, so there is no key-cycling logic:

```c
#include <stdio.h>

int main(void) {
    char msg[] = "hello";
    char key = 'K';

    for (int i = 0; msg[i] != '\0'; i++)
        msg[i] ^= key;               // encrypt
    // msg is now unreadable bytes, so don't print it as text

    for (int i = 0; msg[i] != '\0'; i++)
        msg[i] ^= key;               // decrypt: same operation

    printf("%s\n", msg);             // prints: hello
}
```

The loop `msg[i] ^= key` is the whole idea. Running it twice with the same key undoes it, exactly like `a ^ b ^ b == a`, where `a` is a character and `b` is the key.

One catch: an encrypted byte can happen to be `0`. That would end the string early for the `!= '\0'` check. It doesn't happen with "hello" and `'K'`. For general data, loop over a known length instead of looking for `'\0'`.

I haven't compiled this one either, though it's short enough to check by eye. A single-byte key is the weakest case, since there are only 256 possible keys. It's for understanding XOR, not for protecting anything.

## Me

I see. XOR encrypts the information using the key, and the decoder has to know the key so that the message can be transformed into the original information, right? Can you show how to break a single-byte key?

## Claude

Yes, that's right. The sender and receiver share a secret key, and anyone without it should only see gibberish. The weakness is that a one-byte key has only 256 possible values, so an attacker can try all of them.

## Breaking it by brute force

The attacker only has the ciphertext. They try every key from 0 to 255 and keep the ones where the result looks like readable text (every byte printable).

```c
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main(void) {
    char msg[] = "meet me at the park at noon";
    size_t n = strlen(msg);

    for (size_t i = 0; i < n; i++)
        msg[i] ^= 0x5A;                 // sender encrypts; the attacker never sees 0x5A

    // attacker: has only msg and n
    for (int key = 0; key < 256; key++) {
        int ok = 1;
        for (size_t i = 0; i < n; i++)
            if (!isprint((unsigned char)(msg[i] ^ key))) { ok = 0; break; }

        if (ok) {
            printf("key 0x%02X: ", key);
            for (size_t i = 0; i < n; i++)
                putchar(msg[i] ^ key);
            putchar('\n');
        }
    }
}
```

Expected output is the correct key and its plaintext:

```
key 0x5A: meet me at the park at noon
```

## How it works

- The inner loop decrypts with a guessed key, using the same `^` as before.
- If any decrypted byte isn't a printable character, the guess is wrong and the loop skips to the next key.
- Almost every wrong key produces some unprintable byte, so very few guesses survive.

The longer the message, the more likely only the right key survives. A very short message might leave a few candidates, and you pick the one that reads as real text. The same idea extends to longer repeating keys. You split the ciphertext into groups by key position and break each group as a single-byte key, as I described in the first answer.

I haven't compiled this one, and I haven't confirmed that the output shows exactly one key. Use it only on messages you made yourself, as a learning exercise.

## Me

```

int ok = 1;
        for (size_t i = 0; i < n; i++)
            if (!isprint((unsigned char)(msg[i] ^ key))) {
                ok = 0;
                break;
            }

        if (ok) {
            printf("key 0x%02X: ", key);
            for (size_t i = 0; i < n; i++) putchar(msg[i] ^ key);
            putchar('\n');
        }

```


what does this part mean

## Claude

This is the part that tests one guessed `key` and decides whether it's worth showing. I'll take it line by line.

```c
int ok = 1;
```
`ok` is a flag meaning "this key still looks right." It starts as 1 (true) because we assume the key works until a byte proves otherwise.

```c
for (size_t i = 0; i < n; i++)
    if (!isprint((unsigned char)(msg[i] ^ key))) {
        ok = 0;
        break;
    }
```
This loops over every byte of the ciphertext and does three things:

1. `msg[i] ^ key` decrypts byte `i` using the guessed key.
2. `isprint(...)` returns true if the result is a printable character (letters, digits, punctuation, space). Control characters and most bytes above 127 are not printable.
3. `!` flips that. If the decrypted byte is **not** printable, then:
   - `ok = 0;` marks this key as wrong.
   - `break;` stops checking early, since one bad byte is enough to rule it out.

The `(unsigned char)` cast is there because `char` can be negative on some systems. `isprint` expects a value from 0 to 255, so the cast avoids undefined behavior.

```c
if (ok) {
    printf("key 0x%02X: ", key);
    for (size_t i = 0; i < n; i++) putchar(msg[i] ^ key);
    putchar('\n');
}
```
If `ok` is still 1 after the loop, every decrypted byte was printable. The key is a plausible candidate, so the program does the following:

- `printf("key 0x%02X: ", key)` prints the key in hex, with at least two digits (for example `0x5A`).
- The `for` loop decrypts the message again and prints it one character at a time with `putchar`.
- `putchar('\n')` ends the line.

A key that fails is skipped silently, and a key that passes gets its decrypted text printed so you can read it and decide.

## Me

I ran the code, and below is the output. There are less than 100 lines being printed out, but there should be 256 lines because you tried 256 keys. Why? Is that because some lines are omitted due to information that is not printable?


```

learncli$ ./test
key 0x00: 7??.z7?z;.z.2?z*;(1z;.z4554
key 0x01: 6>>/{6>{:/{/3>{+:)0{:/{5445
key 0x02: 5==,x5=x9,x,0=x(9*3x9,x6776
key 0x03: 4<<-y4<y8-y-1<y)8+2y8-y7667
key 0x04: 3;;*~3;~?*~*6;~.?,5~?*~0110
key 0x06: 199(|19|=(|(49|,=.7|=(|2332
key 0x07: 088)}08}<)})58}-</6}<)}3223
key 0x08: ?77&r?7r3&r&:7r"3 9r3&r<==<
key 0x09: >66's>6s2's';6s#2!8s2's=<<=
key 0x0A: =55$p=5p1$p$85p 1";p1$p>??>
key 0x0B: <44%q<4q0%q%94q!0#:q0%q?>>?
key 0x0C: ;33"v;3v7"v">3v&7$=v7"v8998
key 0x0D: :22#w:2w6#w#?2w'6%<w6#w9889
key 0x0E: 911 t91t5 t <1t$5&?t5 t:;;:
key 0x0F: 800!u80u4!u!=0u%4'>u4!u;::;
key 0x10: '//>j'/j+>j>"/j:+8!j+>j$%%$
key 0x11: &..?k&.k*?k?#.k;*9 k*?k%$$%
key 0x12: %--<h%-h)<h< -h8):#h)<h&''&
key 0x13: $,,=i$,i(=i=!,i9(;"i(=i'&&'
key 0x14: #++:n#+n/:n:&+n>/<%n/:n !! 
key 0x15: "**;o"*o.;o;'*o?.=$o.;o!  !
key 0x16: !))8l!)l-8l8$)l<->'l-8l"##"
key 0x17:  ((9m (m,9m9%(m=,?&m,9m#""#
key 0x18: /''6b/'b#6b6*'b2#0)b#6b,--,
key 0x19: .&&7c.&c"7c7+&c3"1(c"7c-,,-
key 0x1A: -%%4`-%`!4`4(%`0!2+`!4`.//.
key 0x1B: ,$$5a,$a 5a5)$a1 3*a 5a/../
key 0x1C: +##2f+#f'2f2.#f6'4-f'2f())(
key 0x1D: *""3g*"g&3g3/"g7&5,g&3g)(()
key 0x1E: )!!0d)!d%0d0,!d4%6/d%0d*++*
key 0x1F: (  1e( e$1e1- e5$7.e$1e+**+
key 0x41: v~~o;v~;zo;os~;kzip;zo;uttu
key 0x42: u}}l8u}8yl8lp}8hyjs8yl8vwwv
key 0x43: t||m9t|9xm9mq|9ixkr9xm9wvvw
key 0x45: rzzk?rz?~k?kwz?o~mt?~k?qppq
key 0x46: qyyh<qy<}h<hty<l}nw<}h<rssr
key 0x47: pxxi=px=|i=iux=m|ov=|i=srrs
key 0x49: ~vvg3~v3rg3g{v3crax3rg3}||}
key 0x4C: {ssb6{s6wb6b~s6fwd}6wb6xyyx
key 0x4F: xppa5xp5ta5a}p5etg~5ta5{zz{
key 0x50: goo~*go*k~*~bo*zkxa*k~*deed
key 0x52: emm|(em(i|(|`m(xizc(i|(fggf
key 0x53: dll})dl)h})}al)yh{b)h})gffg
key 0x54: ckkz.ck.oz.zfk.~o|e.oz.`aa`
key 0x56: aiix,ai,mx,xdi,|m~g,mx,bccb
key 0x58: oggv"og"cv"vjg"rcpi"cv"lmml
key 0x59: nffw#nf#bw#wkf#sbqh#bw#mllm
key 0x5A: meet me at the park at noon
key 0x5B: lddu!ld!`u!uid!q`sj!`u!onno
key 0x5C: kccr&kc&gr&rnc&vgtm&gr&hiih
key 0x5D: jbbs'jb'fs'sob'wful'fs'ihhi
key 0x5E: iaap$ia$ep$pla$tevo$ep$jkkj
key 0x5F: h``q%h`%dq%qm`%udwn%dq%kjjk
learncli$

```

## Claude

Yes. Any key that produces even one non-printable byte is skipped, and those lines never get printed. So 256 keys were tried, but only the ones that passed the check were printed. Your output has about 51 lines.

I was wrong earlier when I said very few wrong keys would survive. Many survive for this message, for two reasons:

- **Keys 0x80 to 0xFF all fail.** The plaintext and `0x5A` both have the top bit 0, so the ciphertext bytes are all below 128. XORing with a key that has the top bit set produces bytes of 128 or more, which `isprint` rejects.
- **Many low keys pass.** The plaintext is only lowercase letters and spaces, which sit in a range of ASCII (`0x20`–`0x7E`) that is mostly printable. A wrong key that differs from `0x5A` in just a few bits often moves each byte to another printable character. That gives gibberish like `7??.z7?z` that still counts as "printable."

"Printable" is a weak test. The correct key, `0x5A`, is the only line that reads as English, so you pick it by eye.

## A stricter test

You can make the test fit the likely plaintext. This message only has letters and spaces, so require exactly that. Replace the `isprint(...)` line with:

```c
unsigned char c = msg[i] ^ key;
if (!(isalpha(c) || c == ' ')) { ok = 0; break; }
```

Now a key survives only if every decrypted byte is a letter or a space. I expect that to cut the list to `0x5A` alone, but I haven't run it, so check your own output. This works only because I know the message is plain lowercase English. If the real message had digits or punctuation, this filter would reject the correct key too. Real attacks usually score each candidate instead, for example by counting how many bytes are common English letters, and keep the best-scoring key.

## Me

How could some lines be non-printable?

## Claude

A byte is just a number from 0 to 255, and only some of those numbers are characters you can see on screen. `isprint` returns true for ASCII 32 to 126, which covers letters, digits, punctuation and space. Everything else is non-printable:

- **0 to 31:** control characters. They do things instead of showing a symbol: `0` ends a C string, `7` beeps, `9` is a tab, `10` is a newline, and `27` is escape.
- **127:** DEL, another control character.
- **128 to 255:** not part of standard ASCII. Whether they show anything depends on the terminal's encoding, and they usually come out as garbage or `?`.

A wrong key can produce any byte, so it can land in those ranges. Here is a concrete example from your run. The plaintext starts `"meet me..."`, and the space (`0x20`) encrypts to `0x20 ^ 0x5A = 0x7A`. Now try the wrong key `0x05` on that byte:

```
0x7A ^ 0x05 = 0x7F    // DEL, not printable
```

So `isprint` fails and key `0x05` is dropped. That's why `0x05` is missing from your output between `0x04` and `0x06`. Key `0x40` is missing for the same reason. The `e` encrypts to `0x3F`, and `0x3F ^ 0x40 = 0x7F`.

If the program printed the non-printable ones too, you'd see blank gaps, odd symbols, beeps, or a line cut short or broken by a stray newline or tab. That's why the filter keeps them out.

## Me

got it, and the correct message has to be comprehensible, which helps us find the correct key.

## Claude

Yes, that's the core of it. The attacker needs some way to recognize the right answer. Here that means the output is readable English, which is why we could rule out keys that gave non-printable bytes and pick the one line that made sense.

This also explains why the one-time pad is secure. With a truly random key as long as the message, every possible plaintext of that length is equally likely, and each one has a key that produces it. A 5-byte ciphertext could decrypt to "hello" or to "world" with different keys, and nothing tells you which is right. The attacker has nothing to recognize.

With a short, reused key, only one key gives sensible text, so the ciphertext gives it away.

## Me

Although a single character key can make the encryption insecure, we can use a longer key to fortify the encryption. However, you said it is still unsafe, right? Can you show me how to break a longer repeating key and summarize a few reasons that make it insecure?

## Claude

## Breaking a repeating key

With a 3-byte key like `"k3y"`, byte 0, byte 3, byte 6, and so on are all XORed with `'k'`. Byte 1, 4, 7 use `'3'`, and byte 2, 5, 8 use `'y'`. So each "column" of the message is just a single-byte-key cipher. We already know how to break one of those, so we solve each column on its own.

For each column we try all 256 key bytes and score the result by how many decrypted bytes are letters or spaces. The best-scoring byte wins.

```c
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(void) {
    char msg[] = "xor with a short repeating key is weak because every third byte "
                 "of the message is hidden by the same key byte so we can break "
                 "each column on its own just like a single byte key";
    const char *key = "k3y";
    size_t n = strlen(msg), klen = 3;

    for (size_t i = 0; i < n; i++)
        msg[i] ^= key[i % klen];                 // sender encrypts

    // attacker: knows only msg, n, and klen
    for (size_t col = 0; col < klen; col++) {
        int best = 0, bestscore = -1;
        for (int k = 0; k < 256; k++) {
            int score = 0;
            for (size_t i = col; i < n; i += klen) {
                unsigned char c = msg[i] ^ k;
                if (isalpha(c) || c == ' ') score++;
            }
            if (score > bestscore) { bestscore = score; best = k; }
        }
        printf("key byte %zu: '%c'\n", col, best);
        for (size_t i = col; i < n; i += klen)
            msg[i] ^= best;                      // decrypt this column
    }
    printf("%s\n", msg);
}
```

The expected output is the three key bytes `k`, `3`, `y`, followed by the full plaintext.

The loop `for (i = col; i < n; i += klen)` visits every `klen`-th byte starting at `col`, which is exactly one column. I take `n` before encrypting because an encrypted byte can be `0`, and `strlen` would stop there.

The attack needs enough text. Each column needs a few dozen characters for the letters-and-spaces score to single out one key byte. It also assumes we know the key length. If we don't, we try lengths 1, 2, 3, and so on. Real attacks use tricks like measuring how often bytes repeat at a given spacing to find it.

## Why a longer repeating key is still insecure

1. **It reduces to many easy problems.** A key of length L is just L single-byte ciphers interleaved. A longer key only means more columns to solve, not a harder problem.
2. **Plaintext has statistical patterns.** Letters and spaces are not random, and each column keeps those patterns, which is what the scoring exploits. The more text you encrypt with the same key, the more material the attacker gets.
3. **Known plaintext reveals the key.** Since `k = c ^ p`, any guessed part of the message (a file header, `"Dear "`, `"From:"`) gives the key bytes at those positions directly.
4. **Reuse leaks.** Two messages under the same key give `c1 ^ c2 = p1 ^ p2`, and the key cancels out.
5. **There is no integrity check.** Anyone can flip bits in the ciphertext, and the plaintext changes in the same places without any error.

The fix is a key as long as the message, random, and used once (the one-time pad). The other option is to use a real cipher like AES or ChaCha20, which stretch a short key into a keystream that has no such patterns.

I haven't compiled or run this program, so test it. For short messages the best score can tie, and the code keeps the first key byte it tried that reached the top score, which may not be the right one.

## Me

```

learncli$ ./test
key byte 0: 'k'
key byte 1: '3'
key byte 2: 'y'
xor with a short repeating key is weak because every third byte of the message is hidden by the same key byte so we can break each column on its own just like a single byte key

```


I ran the code. It got the key immediately. This is so insecure. You mentioned AES. What is that? Is XOR used in AES? How is that different from what I did?

## Claude

AES (Advanced Encryption Standard) is the block cipher almost everything uses today: HTTPS, Wi-Fi, disk encryption. It takes a 16-byte block of data and a key of 16, 24, or 32 bytes, and scrambles the block into 16 bytes of ciphertext. The US government standardized it in 2001 after a public competition, and decades of attacks haven't broken it.

## Is XOR used in AES?

Yes, it's one of the pieces. In each round, a step called AddRoundKey XORs the 16-byte block with a round key. But XOR is only one of four steps, and it's the only one that uses the key directly.

## How is it different from what you did?

Roughly, AES takes the block through 10 to 14 rounds, and each round does this:

1. **SubBytes:** replace every byte using a lookup table (the S-box). The table is deliberately nonlinear, so the output has no simple XOR-like relationship to the input.
2. **ShiftRows:** shuffle bytes between positions.
3. **MixColumns:** combine the bytes within each column so every output byte depends on several inputs.
4. **AddRoundKey:** XOR with a round key.

The round keys come from a **key schedule** that expands your one key into a different key for each round.

The differences that matter for your attack:

- **Your attack worked because each column was independent.** A byte of plaintext only affected the one ciphertext byte at the same position, so you could solve each column alone. In AES, changing one input bit changes about half the output bits (the avalanche effect), because the mixing steps spread every byte's influence across the whole block. There are no independent columns to attack.
- **The XOR alone is linear and the S-box is not.** Linear structure is what lets you cancel out keys with `c ^ p = k`. The S-box destroys that.
- **The key is used many times in different forms.** There's no repeating pattern like `k3yk3yk3y`.

Designers call these two goals **confusion** (the key and ciphertext relationship is complex) and **diffusion** (each plaintext bit affects many ciphertext bits). Your XOR cipher has neither.

## Two caveats

- AES only encrypts one 16-byte block. For longer data you need a mode, like CTR or GCM, that handles many blocks safely. A bad mode can leak patterns, as ECB mode does, where equal plaintext blocks give equal ciphertext blocks. Several modes (CTR, CBC) use XOR to chain blocks together.
- Don't write your own AES or other crypto for real use. Use a vetted library such as OpenSSL or libsodium. Implementations can be broken by subtle bugs and timing leaks even when the algorithm is sound.

I've simplified a lot here, and if you want to see the real thing, the AES spec (FIPS 197) is short and readable.
