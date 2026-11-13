#include "targets.h"

// Compile this translation unit as C, separately from the harness, without LTO.
// Volatile accesses retain a fixed-address workload instead of a removable delay.
// Unsigned arithmetic makes wraparound defined; no sleeps or RNG are measured.
static uint32_t workload(uint32_t iterations)
{
    volatile uint32_t scratch = 0;
    for (uint32_t i = 0; i < iterations; ++i) {
        scratch = scratch + i + 1u;
    }
    return scratch;
}

uint32_t control_leaky(const uint8_t secret[CONTROL_SECRET_BYTES])
{
    // Deliberately amplified leak: secret bit selects 64 versus 4096 iterations.
    // Optimization may turn selection into arithmetic; the loop bound still leaks.
    uint32_t iterations = (secret[0] & 1u) ? CONTROL_LONG_ITERATIONS
                                          : CONTROL_SHORT_ITERATIONS;
    return workload(iterations) ^ secret[0];
}

uint32_t control_fixed_work(const uint8_t secret[CONTROL_SECRET_BYTES])
{
    // Negative control: public bounds and addresses, same work for both classes.
    // Secret affects only the returned value, never a branch or memory address.
    return workload(CONTROL_LONG_ITERATIONS) ^ secret[0];
}
