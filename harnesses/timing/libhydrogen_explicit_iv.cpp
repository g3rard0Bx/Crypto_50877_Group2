// Dudect fixed/random-key timing experiment using Libhydrogen's internal
// explicit-IV encryption with a fixed public IV. Excludes IV RNG to align more
// closely with Monocypher; analyze separately from public-API timing results.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "hydrogen.h"
#include "hydrogen_experiment.h"

#define DUDECT_IMPLEMENTATION
#include "dudect.h"

#define SECRET_KEY_SIZE hydro_secretbox_KEYBYTES
#define TEXT_SIZE 12
#define SAMPLE_SIZE (SECRET_KEY_SIZE + TEXT_SIZE)

#define CONTEXT "Example"
#define MESSAGE "Lorem ipsum"

// Retain output so measured encryption work remains observable.
static volatile uint8_t output_sink;
// Sticky across every measured call: a later success must not hide a failure.
static volatile unsigned encryption_failed;

// Public laboratory input prepared outside measurement. Reusing a fixed IV is
// for this experiment only, not application encryption guidance.
static const uint8_t experimental_iv[HYDRO_EXPERIMENT_IVBYTES] = {};

static size_t generated_classes[2];

// Perform one Libhydrogen encryption operation.
uint8_t do_one_computation(uint8_t *data)
{
    // Sample layout: key[32], plaintext[12].
    const uint8_t *secret_key = data;
    const uint8_t *msg = data + SECRET_KEY_SIZE;

    uint8_t ciphertext[hydro_secretbox_HEADERBYTES + TEXT_SIZE];

    // Separate internal-API experiment: a fixed public IV removes IV RNG
    // from measurement and better matches Monocypher's fixed public nonce.
    // Keep these results separate from the public-API harness, which includes
    // IV generation. Algorithms and checksum workloads still differ.
    int enc_result = hydro_experiment_encrypt_iv(
        ciphertext, msg, TEXT_SIZE, 0, CONTEXT, secret_key, experimental_iv
    );

    encryption_failed = encryption_failed | (unsigned)(enc_result != 0);
    if (enc_result != 0) {
        // A failing API need not initialize ciphertext. Do not consume it;
        // main rejects the whole run after this batch, even if later calls pass.
        return 0;
    }

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
            "Libhydrogen encryption: mode=%s; max_batches=%zu; "
            "samples_per_batch=500; fixed/random keys; "
            "plaintext_bytes=%d.\n",
            "explicit-iv (internal API; fixed zero IV; no IV RNG)", batches, TEXT_SIZE);

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

    while (completed < batches && !encryption_failed &&
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

    unsigned any_encryption_failed = encryption_failed;

    dudect_free(&ctx);

    if (any_encryption_failed) {
        fprintf(stderr, "Libhydrogen encryption failed; discard this timing run.\n");
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
