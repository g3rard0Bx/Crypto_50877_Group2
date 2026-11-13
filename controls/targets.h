#ifndef CONTROL_TARGETS_H
#define CONTROL_TARGETS_H

#include <stdint.h>

#define CONTROL_SECRET_BYTES 32
#define CONTROL_SHORT_ITERATIONS 64u
#define CONTROL_LONG_ITERATIONS 4096u

#ifdef __cplusplus
extern "C" {
#endif

// Synthetic method controls, not cryptographic routines. Only secret[0] is used.
uint32_t control_leaky(const uint8_t secret[CONTROL_SECRET_BYTES]);
uint32_t control_fixed_work(const uint8_t secret[CONTROL_SECRET_BYTES]);

#ifdef __cplusplus
}
#endif
#endif
