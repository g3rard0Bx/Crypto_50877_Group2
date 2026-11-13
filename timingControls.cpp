#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "control_targets.h"
#include "monocypher_random.h"
#define DUDECT_IMPLEMENTATION
#include "dudect.h"

// Same dudect timer/batch size as the Monocypher harness, separate experiment.
static bool leaky_mode;
static volatile uint32_t output_sink;
static size_t generated_classes[2];

uint8_t do_one_computation(uint8_t *data)
{
    // Mode is public and fixed for the entire process; result is never branched on.
    uint32_t result = leaky_mode ? control_leaky(data) : control_fixed_work(data);
    output_sink = result; // dudect ignores callback returns; retain the full result.
    return (uint8_t)result;
}

void prepare_inputs(dudect_config_t *config, uint8_t *input_data, uint8_t *classes)
{
    // Prepare randomized order and inputs outside the measured region.
    if (monocypher_random(input_data,
                          config->number_measurements * config->chunk_size) != 0 ||
        monocypher_random(classes, config->number_measurements) != 0) {
        perror("Control input randomness failed");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < config->number_measurements; ++i) {
        classes[i] &= 1u;
        uint8_t *secret = input_data + i * config->chunk_size;
        if (classes[i] == 0) {
            memset(secret, 0, config->chunk_size);
        } else {
            // Deliberately separated control classes, not natural random keys:
            // force the selector bit to 1 so class 1 always takes the long path.
            secret[0] |= 1u;
        }
        ++generated_classes[classes[i]];
    }
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3 ||
        (strcmp(argv[1], "leaky") != 0 && strcmp(argv[1], "fixed-work") != 0)) {
        fprintf(stderr, "Usage: %s leaky|fixed-work [batches: 1..100000]\n", argv[0]);
        return 2;
    }
    leaky_mode = strcmp(argv[1], "leaky") == 0;
    size_t batches = 1;
    if (argc == 3) {
        char *end;
        errno = 0;
        unsigned long value = strtoul(argv[2], &end, 10);
        if (errno != 0 || argv[2][0] < '0' || argv[2][0] > '9' ||
            *end != '\0' || value == 0 || value > 100000) {
            fprintf(stderr, "Batch count must be an integer from 1 to 100000.\n");
            return 2;
        }
        batches = (size_t)value;
    }

    printf("Control=%s; max_batches=%zu; samples_per_batch=500; "
           "iterations=%u/%u; controls not yet validated.\n",
           argv[1], batches, CONTROL_SHORT_ITERATIONS, CONTROL_LONG_ITERATIONS);
    dudect_config_t config = {};
    config.chunk_size = CONTROL_SECRET_BYTES;
    config.number_measurements = 500;
    dudect_ctx_t ctx;
    if (dudect_init(&ctx, &config) != 0) {
        fprintf(stderr, "dudect initialization failed.\n");
        return 1;
    }
    dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;
    size_t completed = 0;
    while (completed < batches && state == DUDECT_NO_LEAKAGE_EVIDENCE_YET) {
        state = dudect_main(&ctx);
        ++completed;
    }
    // Generated counts include warm-up; uncropped counts reflect dudect discards.
    printf("Completed batches=%zu; generated classes=%zu/%zu; "
           "uncropped analyzed classes=%.0f/%.0f.\n",
           completed, generated_classes[0], generated_classes[1],
           ctx.ttest_ctxs[0]->n[0], ctx.ttest_ctxs[0]->n[1]);
    dudect_free(&ctx);
    if (state == DUDECT_LEAKAGE_FOUND) {
        puts("Finding reported; repeat and validate before drawing conclusions.");
        return 3;
    }
    puts("Budget exhausted without a finding; inspect analyzed counts. "
         "Insufficient samples are inconclusive, not proof of constant time.");
    return 0;
}
