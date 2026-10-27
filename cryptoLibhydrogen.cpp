
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <stdint.h>
#include <string.h>
#include <valgrind/memcheck.h>

#include "hydrogen.h"

#define CONTEXT "Example"
#define MESSAGE "test"
#define MESSAGE_LEN (sizeof(MESSAGE))
#define CIPHERTEXT_LEN (hydro_secretbox_HEADERBYTES + MESSAGE_LEN)

int valgrind_phase(uint8_t secret_key[hydro_secretbox_KEYBYTES])
{
    uint8_t msg[] = MESSAGE;
    uint8_t ciphertext[CIPHERTEXT_LEN];
    uint8_t decrypted[MESSAGE_LEN];

    // Mark the secret key and plaintext as undefined
    VALGRIND_MAKE_MEM_UNDEFINED(secret_key, hydro_secretbox_KEYBYTES);
    VALGRIND_MAKE_MEM_UNDEFINED(msg, sizeof(msg));

    // Encrypt the message
    int enc_result = hydro_secretbox_encrypt(
        ciphertext,
        msg,
        sizeof(msg),
        0,
        CONTEXT,
        secret_key
    );

    // Define status before handling the result
    VALGRIND_MAKE_MEM_DEFINED(secret_key, hydro_secretbox_KEYBYTES);
    VALGRIND_MAKE_MEM_DEFINED(msg, sizeof(msg));
    VALGRIND_MAKE_MEM_DEFINED(ciphertext, sizeof(ciphertext));

    if (enc_result != 0) {
        fprintf(stderr, "Libhydrogen encryption failed.\n");
        return 1;
    }

    // Mark the key and ciphertext undefined for the decryption analysis
    VALGRIND_MAKE_MEM_UNDEFINED(secret_key, hydro_secretbox_KEYBYTES);
    VALGRIND_MAKE_MEM_UNDEFINED(ciphertext, sizeof(ciphertext));

    // Decrypt and authenticate the ciphertext
    int dec_result = hydro_secretbox_decrypt(
        decrypted,
        ciphertext,
        sizeof(ciphertext),
        0,
        CONTEXT,
        secret_key
    );

    // Define status before handling the result
    VALGRIND_MAKE_MEM_DEFINED(
        secret_key, hydro_secretbox_KEYBYTES);
    VALGRIND_MAKE_MEM_DEFINED(
        ciphertext, sizeof(ciphertext));
    VALGRIND_MAKE_MEM_DEFINED(decrypted, sizeof(decrypted));

    if (dec_result != 0) {
        fprintf(stderr, "Libhydrogen decryption failed.\n");
        return 1;
    }

    // Confirm that decryption recovered the original message
    if (memcmp(msg, decrypted, sizeof(msg)) != 0) {
        fprintf(stderr, "Decrypted message does not match.\n");
        return 1;
    }

    printf("Libhydrogen encryption/decryption succeeded.\n");
    return 0;
}

int main(void)
{
    if (hydro_init() != 0) {
        fprintf(stderr, "Libhydrogen initialization failed.\n");
        return 1;
    }

    uint8_t key[hydro_secretbox_KEYBYTES];
    hydro_secretbox_keygen(key);

    int result = valgrind_phase(key);

    // Clear the key before exiting
    hydro_wipe(key, sizeof(key));

    return result;
}
