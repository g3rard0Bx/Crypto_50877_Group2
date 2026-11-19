// Compile the unmodified vendor implementation in this translation unit so the
// adapter can call its static explicit-IV helper. Link this object INSTEAD OF
// hydrogen.o; linking both would duplicate the public library symbols.
#include "../third_party/libhydrogen/hydrogen.c"
#include "hydrogen_experiment.h"

int hydro_experiment_encrypt_iv(uint8_t *ciphertext, const void *message,
                                size_t message_size, uint64_t message_id,
                                const char context[hydro_secretbox_CONTEXTBYTES],
                                const uint8_t key[hydro_secretbox_KEYBYTES],
                                const uint8_t iv[HYDRO_EXPERIMENT_IVBYTES])
{
    COMPILER_ASSERT(HYDRO_EXPERIMENT_IVBYTES == hydro_secretbox_IVBYTES);
    return hydro_secretbox_encrypt_iv(ciphertext, message, message_size,
                                      message_id, context, key, iv);
}
