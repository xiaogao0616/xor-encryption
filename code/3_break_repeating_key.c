// 3. Break a repeating XOR key: split the ciphertext into columns
//    (one per key byte) and break each column like a single-byte key.
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN 500

// Score one guessed key byte for one column (every keylen-th byte starting at start)
int score_column(unsigned char *c, int len, int start, int keylen, int key) {
    int score = 0;
    for (int i = start; i < len; i += keylen) {
        unsigned char p = c[i] ^ key;
        if (isalpha(p) || p == ' ')
            score++;
    }
    return score;
}

int main(void) {
    // --- Sender side: encrypt with a secret repeating key ---
    unsigned char msg[MAXLEN] =
        "Meet me at the library at noon. Bring your notes from the "
        "systems class and the homework on bitwise operators. We will "
        "study until the evening and then get dinner together";
    int len = strlen((char *)msg);
    unsigned char secret[] = "SECRET";
    int secretlen = strlen((char *)secret);   // let C count it so it can't get out of sync
    for (int i = 0; i < len; i++)
        msg[i] ^= secret[i % secretlen];

    // --- Attacker side: only has msg (the ciphertext) ---
    for (int keylen = 1; keylen <= 8; keylen++) {
        unsigned char key[8];
        int total = 0;

        // Each column is a single-byte XOR: break it with the old attack
        for (int col = 0; col < keylen; col++) {
            int best_key = 0, best_score = -1;
            for (int k = 0; k < 256; k++) {
                int s = score_column(msg, len, col, keylen, k);
                if (s > best_score) { best_score = s; best_key = k; }
            }
            key[col] = best_key;
            total += best_score;
        }

        printf("keylen %d: score %3d/%d  key = ", keylen, total, len);
        for (int i = 0; i < keylen; i++)
            printf("%c", isprint(key[i]) ? key[i] : '?');
        printf("\n");
    }
    return 0;
}
