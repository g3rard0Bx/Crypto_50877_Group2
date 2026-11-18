
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "hydrogen.h"

#define DUDECT_IMPLEMENTATION
#include "dudect.h"

#define SECRET_KEY_SIZE hydro_secretbox_KEYBYTES
#define TEXT_SIZE 12
#define SAMPLE_SIZE (SECRET_KEY_SIZE + TEXT_SIZE)

#define CONTEXT "Example"
#define MESSAGE "Lorem ipsum"

// Retain output so measured encryption work remains observable.
static volatile uint8_t output_sink;
static volatile int encryption_status;

static size_t generated_classes[2];

// Perform one Libhydrogen encryption operation.
uint8_t do_one_computation(uint8_t *data)
{
    // Sample layout: key[32], plaintext[12].
    const uint8_t *secret_key = data;
    const uint8_t *msg = data + SECRET_KEY_SIZE;

    uint8_t ciphertext[hydro_secretbox_HEADERBYTES + TEXT_SIZE];

    int enc_result = hydro_secretbox_encrypt(
        ciphertext,
        msg,
        TEXT_SIZE,
        0,
        CONTEXT,
        secret_key
    );

    encryption_status = enc_result;

    // Consume every ciphertext byte.
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(ciphertext); ++i) {
        checksum ^= ciphertext[i];
    }

    output_sink = checksum;
    return checksum;
}

// Generate inputs outside the measured region.
void prepare_inputs(
    dudect_config_t *config,
    uint8_t *input_data,
    uint8_t *classes
)
{
    size_t total_input_size =
        config->number_measurements * config->chunk_size;

    hydro_random_buf(input_data, total_input_size);
    hydro_random_buf(classes, config->number_measurements);

    for (size_t i = 0; i < config->number_measurements; ++i) {
        uint8_t *sample =
            input_data + i * config->chunk_size;

        classes[i] &= 1;

        if (classes[i] == 0) {
            memset(sample, 0, SECRET_KEY_SIZE);
        }

        // Keep public plaintext identical across both classes.
        memcpy(
            sample + SECRET_KEY_SIZE,
            MESSAGE,
            TEXT_SIZE
        );

        ++generated_classes[classes[i]];
    }
}

int main(int argc, char **argv)
{
    // Bounded runs support smoke tests and controlled experiments.
    size_t batches = 1;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [batches: 1..100000]\n", argv[0]);
        return 2;
    }

    if (argc == 2) {
        char *end;
        errno = 0;

        unsigned long value = strtoul(argv[1], &end, 10);

        if (errno != 0 ||
            argv[1][0] < '0' || argv[1][0] > '9' ||
            *end != '\0' ||
            value == 0 || value > 100000) {

            fprintf(stderr,
                    "Batch count must be an integer from 1 to 100000.\n");
            return 2;
        }

        batches = (size_t)value;
    }

    // Initialize Libhydrogen before generating inputs or measuring.
    if (hydro_init() != 0) {
        fprintf(stderr, "Libhydrogen initialization failed.\n");
        return 1;
    }

    fprintf(stderr,
            "Libhydrogen encryption: max_batches=%zu; "
            "samples_per_batch=500; fixed/random keys; "
            "plaintext_bytes=%d.\n",
            batches, TEXT_SIZE);

    dudect_config_t config = {};
    config.chunk_size = SAMPLE_SIZE;
    config.number_measurements = 500;

    dudect_ctx_t ctx;

    if (dudect_init(&ctx, &config) != 0) {
        fprintf(stderr, "dudect initialization failed.\n");
        return 1;
    }

    dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;
    size_t completed = 0;

    while (completed < batches &&
           state == DUDECT_NO_LEAKAGE_EVIDENCE_YET) {

        state = dudect_main(&ctx);
        ++completed;
    }

    printf(
        "Completed batches=%zu; generated classes=%zu/%zu; "
        "uncropped analyzed classes=%.0f/%.0f.\n",
        completed,
        generated_classes[0],
        generated_classes[1],
        ctx.ttest_ctxs[0]->n[0],
        ctx.ttest_ctxs[0]->n[1]
    );

    int final_encryption_status = encryption_status;

    dudect_free(&ctx);

    if (final_encryption_status != 0) {
        fprintf(stderr, "Libhydrogen encryption failed.\n");
        return 1;
    }

    if (state == DUDECT_LEAKAGE_FOUND) {
        fprintf(stderr,
                "Timing distinguisher reported; "
                "controls and reproduction required.\n");
        return 3;
    }

    puts(
        "Budget exhausted without a dudect finding; "
        "inspect analyzed counts. "
        "This is not proof of constant time."
    );

    return 0;
}