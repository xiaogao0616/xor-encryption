// 2. Break a single-byte XOR key by trying all 256 keys
//    and scoring each result by how many letters and spaces it has.
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(void) {
    // --- Sender side: encrypt with a secret key ---
    char msg[] = "MEET ME AT THE LIBRARY AT NOON";
    int len = strlen(msg);
    char secret = 77;
    for (int i = 0; i < len; i++)
        msg[i] = msg[i] ^ secret;

    // 'M' is 77 in ASCII, so 'M' ^ 77 = 0: the ciphertext starts with a '\0'
    printf("strlen of ciphertext = %d (real length is %d)\n\n", (int)strlen(msg), len);

    // --- Attacker side: only has msg (the ciphertext) ---
    int best_key = 0, best_score = -1;

    printf("Keys scoring 24 or more:\n");
    for (int key = 0; key < 256; key++) {
        int score = 0;
        for (int i = 0; i < len; i++) {
            char c = msg[i] ^ key;
            if (isalpha((unsigned char)c) || c == ' ')
                score++;
        }
        if (score >= 24) {
            printf("key %3d: score %2d: ", key, score);
            for (int i = 0; i < len; i++) {
                char c = msg[i] ^ key;
                putchar(isprint((unsigned char)c) ? c : '.');   // show non-printable bytes as .
            }
            putchar('\n');
        }
        if (score > best_score) {
            best_score = score;
            best_key = key;
        }
    }

    // Decrypt with the best guess
    printf("\nBest key: %d (score %d/%d)\n", best_key, best_score, len);
    for (int i = 0; i < len; i++)
        msg[i] = msg[i] ^ best_key;
    printf("Message: %s\n", msg);
    return 0;
}
