#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hydrogen.h"

#define CONTEXT "Example"
#define MESSAGE "test"

int main(void)
{
    if (hydro_init() != 0) {
        fprintf(stderr, "Libhydrogen initialization failed.\n");
        return 1;
    }

    uint8_t key[hydro_secretbox_KEYBYTES];
    uint8_t ciphertext[hydro_secretbox_HEADERBYTES + sizeof(MESSAGE)];
    uint8_t decrypted[sizeof(MESSAGE)];

    hydro_secretbox_keygen(key);

    if (hydro_secretbox_encrypt(
            ciphertext, MESSAGE, sizeof(MESSAGE),
            0, CONTEXT, key) != 0) {
        fprintf(stderr, "Encryption failed.\n");
        return 1;
    }

    if (hydro_secretbox_decrypt(
            decrypted, ciphertext, sizeof(ciphertext),
            0, CONTEXT, key) != 0) {
        fprintf(stderr, "Decryption failed.\n");
        return 1;
    }

    if (memcmp(decrypted, MESSAGE, sizeof(MESSAGE)) != 0) {
        fprintf(stderr, "Message mismatch.\n");
        return 1;
    }

    printf("Libhydrogen encryption/decryption succeeded.\n");
    return 0;
}
