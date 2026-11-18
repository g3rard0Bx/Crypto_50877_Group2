#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "monocypher.h"
#include "random.h"
#define DUDECT_IMPLEMENTATION
#include "dudect.h"

#define SECRET_KEY_SIZE 32
#define NONCE_SIZE 24
#define TEXT_SIZE 12
#define MAC_SIZE 16
#define SAMPLE_SIZE (SECRET_KEY_SIZE + NONCE_SIZE + TEXT_SIZE)

// Added: retain every output byte so the measured work is observable even if
// optimization/inlining changes. This common sink adds overhead to both classes.
static volatile uint8_t output_sink;
static size_t generated_classes[2];

uint8_t do_one_computation(uint8_t *data)
{
    // Replaced the unfinished check_tag example with actual AEAD encryption.
    // Sample layout: key[32], public nonce[24], public plaintext[12].
    uint8_t cipher_text[TEXT_SIZE];
    uint8_t mac[MAC_SIZE];
    crypto_aead_lock(cipher_text, mac, data, data + SECRET_KEY_SIZE,
                     NULL, 0, data + SECRET_KEY_SIZE + NONCE_SIZE, TEXT_SIZE);
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(cipher_text); ++i) {
        checksum ^= cipher_text[i];
    }
    for (size_t i = 0; i < sizeof(mac); ++i) {
        checksum ^= mac[i];
    }
    output_sink = checksum;
    return checksum;
}

void prepare_inputs(dudect_config_t *config, uint8_t *input_data, uint8_t *classes)
{
    // Added: all randomness and class preparation happen outside measurement.
    // Abort on RNG failure because dudect's preparation callback returns void.
    if (monocypher_random(input_data,
                          config->number_measurements * config->chunk_size) != 0 ||
        monocypher_random(classes, config->number_measurements) != 0) {
        perror("Monocypher timing-input randomness failed");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < config->number_measurements; ++i) {
        uint8_t *sample = input_data + i * config->chunk_size;
        classes[i] &= 1;
        if (classes[i] == 0) {
            memset(sample, 0, SECRET_KEY_SIZE); // fixed versus random key
        }
        // Match public inputs across classes. Repeated nonces are intentional
        // laboratory inputs here, never a recipe for application encryption.
        memset(sample + SECRET_KEY_SIZE, 0, NONCE_SIZE);
        memcpy(sample + SECRET_KEY_SIZE + NONCE_SIZE, "Lorem ipsum", TEXT_SIZE);
        ++generated_classes[classes[i]];
    }
}

int main(int argc, char **argv)
{
    // Bounded runs support smoke checks and predeclared pilot budgets. Controls
    // must be validated separately for the selected compiler/environment.
    size_t batches = 1;
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [batches: 1..100000]\n", argv[0]);
        return 2;
    }
    if (argc == 2) {
        char *end;
        errno = 0;
        unsigned long value = strtoul(argv[1], &end, 10);
        if (errno != 0 || argv[1][0] < '0' || argv[1][0] > '9' ||
            *end != '\0' || value == 0 || value > 100000) {
            fprintf(stderr, "Batch count must be an integer from 1 to 100000.\n");
            return 2;
        }
        batches = (size_t)value;
    }
    fprintf(stderr, "Monocypher encryption: max_batches=%zu; samples_per_batch=500; "
                    "fixed/random keys; plaintext_bytes=%d.\n", batches, TEXT_SIZE);
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
        fprintf(stderr, "Timing distinguisher reported; controls and reproduction required.\n");
        return 3;
    }
    puts("Budget exhausted without a dudect finding; inspect analyzed counts. "
         "This is not proof of constant time.");
    return 0;
}
