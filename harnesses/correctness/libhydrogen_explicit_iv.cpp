#include <stdio.h>
#include <string.h>
#include "hydrogen_experiment.h"

// Verify the internal explicit-IV path independently of dudect. Fixed-IV
// determinism is needed for the matched-input experiment, not an application
// recommendation. Public-API correctness remains a separate executable.
int main()
{
    if (hydro_init() != 0) return 1;
    const char context[hydro_secretbox_CONTEXTBYTES] = "Example";
    const char message[] = "Lorem ipsum";
    uint8_t key[hydro_secretbox_KEYBYTES] = {};
    uint8_t iv[HYDRO_EXPERIMENT_IVBYTES] = {};
    uint8_t ciphertext[hydro_secretbox_HEADERBYTES + sizeof message];
    uint8_t repeated[sizeof ciphertext], modified[sizeof ciphertext];
    uint8_t zero_iv_ciphertext[sizeof ciphertext];
    uint8_t plaintext[sizeof message];
    for (unsigned i = 0; i < 4; ++i) {
        // Cover fixed/random keys and fixed/nonzero IVs outside timing.
        if (i & 1) hydro_secretbox_keygen(key); else memset(key, 0, sizeof key);
        memset(iv, i >= 2 ? 0x5a : 0, sizeof iv);
        if (hydro_experiment_encrypt_iv(ciphertext, message, sizeof message,
                                       0, context, key, iv) != 0 ||
            hydro_experiment_encrypt_iv(repeated, message, sizeof message,
                                       0, context, key, iv) != 0 ||
            memcmp(ciphertext, repeated, sizeof ciphertext) != 0 ||
            hydro_secretbox_decrypt(plaintext, ciphertext, sizeof ciphertext,
                                    0, context, key) != 0 ||
            memcmp(plaintext, message, sizeof message) != 0) {
            fputs("FAIL: explicit-IV determinism or round trip\n", stderr);
            return 1;
        }
        if (i == 0) memcpy(zero_iv_ciphertext, ciphertext, sizeof ciphertext);
        if (i == 2 && memcmp(zero_iv_ciphertext, ciphertext, sizeof ciphertext) == 0) {
            fputs("FAIL: supplied IV was ignored\n", stderr);
            return 1;
        }
        const size_t indices[] = {0, hydro_secretbox_HEADERBYTES - 1,
                                  hydro_secretbox_HEADERBYTES};
        for (size_t index : indices) {
            memcpy(modified, ciphertext, sizeof ciphertext);
            modified[index] ^= 1;
            if (hydro_secretbox_decrypt(plaintext, modified, sizeof modified,
                                        0, context, key) == 0) {
                fputs("FAIL: modified header/payload accepted\n", stderr);
                return 1;
            }
        }
        key[0] ^= 1;
        if (hydro_secretbox_decrypt(plaintext, ciphertext, sizeof ciphertext,
                                    0, context, key) == 0) {
            fputs("FAIL: wrong key accepted\n", stderr);
            return 1;
        }
    }
    puts("PASS: explicit-IV determinism, round trips, header/payload tamper and wrong-key rejection.");
    return 0;
}
