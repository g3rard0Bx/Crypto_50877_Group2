#include <stdio.h>
#include <string.h>
#include <valgrind/memcheck.h>

#include "control_targets.h"

// Separate correctness/annotation driver: never use this program for timing.
static int correctness(void)
{
    uint8_t secret[CONTROL_SECRET_BYTES] = {};
    for (unsigned value = 0; value < 256; ++value) {
        secret[0] = (uint8_t)value;
        uint32_t iterations = (value & 1u) ? CONTROL_LONG_ITERATIONS
                                           : CONTROL_SHORT_ITERATIONS;
        // Independent closed-form oracle for the loop's sum, then output XOR.
        uint32_t leaky_expected = (iterations * (iterations + 1u) / 2u) ^ value;
        uint32_t fixed_expected =
            (CONTROL_LONG_ITERATIONS * (CONTROL_LONG_ITERATIONS + 1u) / 2u) ^ value;
        if (control_leaky(secret) != leaky_expected ||
            control_fixed_work(secret) != fixed_expected) {
            fprintf(stderr, "FAIL: control result for input byte %u\n", value);
            return 1;
        }
    }
    puts("PASS: both control targets match expected results for all 256 selector bytes.");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) {
        return correctness();
    }
    if (argc != 2 ||
        (strcmp(argv[1], "leaky") != 0 && strcmp(argv[1], "fixed-work") != 0)) {
        fprintf(stderr, "Usage: %s [leaky|fixed-work]\n", argv[0]);
        return 2;
    }
    const bool leaky = strcmp(argv[1], "leaky") == 0; // Public selection.
    volatile uint32_t sink = 0;
    for (unsigned bit = 0; bit < 2; ++bit) {
        uint8_t secret[CONTROL_SECRET_BYTES] = {};
        secret[0] = (uint8_t)bit;
        // Only the secret input is poisoned; loop bounds and mode remain public.
        VALGRIND_MAKE_MEM_UNDEFINED(secret, sizeof(secret));
        uint32_t result = leaky ? control_leaky(secret) : control_fixed_work(secret);
        // Result is intentionally observable. This cannot suppress any report
        // already raised inside the target. Avoid reporting secret output in I/O.
        VALGRIND_MAKE_MEM_DEFINED(&result, sizeof(result));
        VALGRIND_MAKE_MEM_DEFINED(secret, sizeof(secret));
        sink = result;
    }
    (void)sink;
    puts("Annotated target calls completed for both selector bits; inspect Memcheck reports.");
    return 0;
}
