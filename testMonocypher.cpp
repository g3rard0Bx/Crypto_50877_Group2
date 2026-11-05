#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "monocypher.h"
#include "monocypher_random.h"

// Added: ordinary correctness checks, deliberately without secret annotations.
// Each negative case changes exactly one input and keeps the others valid.
int main(void)
{
    uint8_t key[32];
    uint8_t nonce[24];
    if (monocypher_random(key, sizeof(key)) != 0 ||
        monocypher_random(nonce, sizeof(nonce)) != 0) {
        perror("Monocypher correctness randomness failed");
        crypto_wipe(key, sizeof(key));
        return 1;
    }

    const uint8_t plain_text[] = "Lorem ipsum";
    uint8_t cipher_text[sizeof(plain_text)];
    uint8_t decrypted[sizeof(plain_text)];
    uint8_t mac[16];
    int result = 0;
    crypto_aead_lock(cipher_text, mac, key, nonce, NULL, 0,
                     plain_text, sizeof(plain_text));

    // Added: explicit checks stay active when assertions are disabled.
    if (crypto_aead_unlock(decrypted, mac, key, nonce, NULL, 0,
                           cipher_text, sizeof(cipher_text)) != 0 ||
        memcmp(decrypted, plain_text, sizeof(plain_text)) != 0) {
        fprintf(stderr, "FAIL: encryption/decryption round trip\n");
        result = 1;
    } else {
        puts("PASS: encryption/decryption round trip and plaintext equality");
    }

    cipher_text[0] ^= 1;
    if (crypto_aead_unlock(decrypted, mac, key, nonce, NULL, 0,
                           cipher_text, sizeof(cipher_text)) == 0) {
        fprintf(stderr, "FAIL: modified ciphertext accepted\n");
        result = 1;
    } else {
        puts("PASS: modified ciphertext rejected");
    }
    cipher_text[0] ^= 1; // Restore before testing the next independent change.

    mac[0] ^= 1;
    if (crypto_aead_unlock(decrypted, mac, key, nonce, NULL, 0,
                           cipher_text, sizeof(cipher_text)) == 0) {
        fprintf(stderr, "FAIL: modified authentication tag accepted\n");
        result = 1;
    } else {
        puts("PASS: modified authentication tag rejected");
    }
    mac[0] ^= 1;

    key[0] ^= 1;
    if (crypto_aead_unlock(decrypted, mac, key, nonce, NULL, 0,
                           cipher_text, sizeof(cipher_text)) == 0) {
        fprintf(stderr, "FAIL: wrong key accepted\n");
        result = 1;
    } else {
        puts("PASS: wrong key rejected");
    }

    // Never inspect decrypted output from a rejected authentication attempt.
    crypto_wipe(key, sizeof(key));
    crypto_wipe(decrypted, sizeof(decrypted));
    return result;
}
