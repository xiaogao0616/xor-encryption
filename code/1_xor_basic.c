// 1. XOR undoes itself: the same loop encrypts and decrypts.
#include <stdio.h>
#include <string.h>

int main(void) {
    char msg[] = "HELLO";
    char key = 42;
    int len = strlen(msg);   // save the length now: ciphertext may contain a 0 byte

    // Encrypt: XOR every character with the key
    for (int i = 0; i < len; i++)
        msg[i] = msg[i] ^ key;

    // Ciphertext is binary data, so print it as hex, not as a string
    printf("Encrypted: ");
    for (int i = 0; i < len; i++)
        printf("%02x ", (unsigned char)msg[i]);
    printf("\n");

    // Decrypt: do the exact same thing again
    for (int i = 0; i < len; i++)
        msg[i] = msg[i] ^ key;

    printf("Decrypted: %s\n", msg);
    return 0;
}
