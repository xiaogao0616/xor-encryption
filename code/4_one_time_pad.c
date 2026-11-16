// 4. One-time pad: with a random key as long as the message, used once,
//    the same ciphertext can "decrypt" to any message of that length.
#include <stdio.h>

int main(void) {
    char msg[]  = "ATTACK";
    unsigned char key[] = {0x3f, 0xa2, 0x91, 0x07, 0xd4, 0x5c};  // random, used once
    char fake[] = "DEFEND";
    unsigned char c[6], fakekey[6];
    char out1[7] = {0}, out2[7] = {0};

    for (int i = 0; i < 6; i++) c[i] = msg[i] ^ key[i];          // encrypt

    printf("Ciphertext:   ");
    for (int i = 0; i < 6; i++) printf("%02x ", c[i]);
    printf("\n");

    for (int i = 0; i < 6; i++) out1[i] = c[i] ^ key[i];         // real key
    for (int i = 0; i < 6; i++) fakekey[i] = c[i] ^ fake[i];     // attacker's "key": k = c ^ p
    for (int i = 0; i < 6; i++) out2[i] = c[i] ^ fakekey[i];

    printf("Real key  -> %s\n", out1);
    printf("Other key -> %s\n", out2);
    return 0;
}
