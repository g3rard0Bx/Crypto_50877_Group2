#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <valgrind/memcheck.h>

#include "monocypher.h"
#include "monocypher_random.h"

// Maintained Monocypher Memcheck starting point. Correctness and timing
// experiments will be maintained separately as the harnesses are expanded.
int valgrind_impl(void)
{
    uint8_t key[32];
    uint8_t nonce[24];
    uint8_t plain_text[12] = "Lorem ipsum";
    uint8_t mac[16];
    uint8_t cipher_text[sizeof(plain_text)];
    uint8_t decrypted[sizeof(plain_text)];

    // Added: check randomness before using either buffer.
    if (monocypher_random(key, sizeof(key)) != 0 ||
        monocypher_random(nonce, sizeof(nonce)) != 0) {
        perror("Monocypher randomness failed");
        crypto_wipe(key, sizeof(key));
        return 1;
    }

    // Changed: this experiment treats only the key as secret. Plaintext,
    // nonce, lengths, ciphertext, and supplied tag remain public/defined.
    VALGRIND_MAKE_MEM_UNDEFINED(key, sizeof(key));
    crypto_aead_lock(cipher_text, mac, key, nonce, NULL, 0,
                     plain_text, sizeof(plain_text));

    // Added: declassify published encryption outputs before decryption.
    VALGRIND_MAKE_MEM_DEFINED(cipher_text, sizeof(cipher_text));
    VALGRIND_MAKE_MEM_DEFINED(mac, sizeof(mac));

    int status = crypto_aead_unlock(decrypted, mac, key, nonce, NULL, 0,
                                    cipher_text, sizeof(cipher_text));

    // Authentication success/failure is observable. Defining status here
    // does not suppress reports already raised inside crypto_aead_unlock().
    VALGRIND_MAKE_MEM_DEFINED(&status, sizeof(status));
    VALGRIND_MAKE_MEM_DEFINED(key, sizeof(key));
    VALGRIND_MAKE_MEM_DEFINED(decrypted, sizeof(decrypted));

    // Added: explicit checks survive NDEBUG, unlike the previous assert.
    int result = 0;
    if (status != 0) {
        fprintf(stderr, "Monocypher authentication failed.\n");
        result = 1;
    } else if (memcmp(plain_text, decrypted, sizeof(plain_text)) != 0) {
        fprintf(stderr, "Monocypher plaintext mismatch.\n");
        result = 1;
    } else {
        puts("Monocypher encryption/decryption succeeded.");
    }

    crypto_wipe(key, sizeof(key));
    crypto_wipe(plain_text, sizeof(plain_text));
    crypto_wipe(decrypted, sizeof(decrypted));
    return result;
}

int main(void)
{
    // Fixed: propagate the harness result instead of a bare return in int main.
    return valgrind_impl();
}
