#ifndef HYDROGEN_EXPERIMENT_H
#define HYDROGEN_EXPERIMENT_H

#include "hydrogen.h"

// Internal IV size in this vendored revision, checked by the C adapter below.
#define HYDRO_EXPERIMENT_IVBYTES 20

#ifdef __cplusplus
extern "C" {
#endif
// Experimental interface, not a supported Libhydrogen public API.
int hydro_experiment_encrypt_iv(uint8_t *ciphertext, const void *message,
                                size_t message_size, uint64_t message_id,
                                const char context[hydro_secretbox_CONTEXTBYTES],
                                const uint8_t key[hydro_secretbox_KEYBYTES],
                                const uint8_t iv[HYDRO_EXPERIMENT_IVBYTES]);
#ifdef __cplusplus
}
#endif
#endif
