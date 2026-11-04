#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/random.h>

#include "monocypher.h"
#define DUDECT_IMPLEMENTATION
#include "dudect.h"

#define SECRET_KEY_SIZE 32
#define NONCE_SIZE      24
#define TEXT_SIZE       12  
#define MAC_SIZE        16


// dudect




/*

    functions to implement:

    do_one_computation()

    prepare_inputs()

    call dudect_main() from main function

*/

/*(Example Below)*/

 

/* target function to check for constant time */

int check_tag(uint8_t *x, uint8_t *y, size_t len) {

//   return memcmp(x, y, len);

 

}

 

// change?

#define SECRET_LEN_BYTES (512)

uint8_t secret[SECRET_LEN_BYTES] = {0, 1, 2, 3, 4, 5, 6, 42};

 

/* this will be called over and over */

uint8_t do_one_computation(uint8_t *data) {

  /* simulate totally bogus MAC check in non-constant time */

  return check_tag(data, secret, SECRET_LEN_BYTES);

}

 

/* called once per number_measurements */

void prepare_inputs(dudect_config_t *c, uint8_t *input_data, uint8_t *classes) {

  randombytes(input_data, c->number_measurements * c->chunk_size);

 

  for (size_t i = 0; i < c->number_measurements; i++) {

    /* it is important to randomize the class sequence */

    classes[i] = randombit();

    if (classes[i] == 0) {

      memset(input_data + (size_t)i * c->chunk_size, 0x00, c->chunk_size);

    } else {

      // leave random

    }

  }

}

 

int run_test(void) {

  dudect_config_t config = {

      .chunk_size = SECRET_LEN_BYTES,

      .number_measurements = 500,

  };

  dudect_ctx_t ctx;

 

  dudect_init(&ctx, &config);

 

  /*

  Call dudect_main() until

   - returns something different than DUDECT_NO_LEAKAGE_EVIDENCE_YET, or

   - you spent too much time testing and give up

  Recommended that you wrap this program with timeout(2) if you don't

  have infinite time.

  For example this will run for 20 mins:

    $ timeout 1200 ./your-executable

  */

  dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;

  while (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET) {

    state = dudect_main(&ctx);

  }

  dudect_free(&ctx);

  return (int)state;

}

 

/*(END Example ABOVE)*/

/*

MONOCYPHER DUDECT IMPL?

*/

 

/*

the function crypto_aead_lock accepts key, nonce, and msg

therefore we need to unpack the data into their respective destination

Example:

key = 32 bytes

key = chunck

nonce = 24 bytes

nonce = chunck + 32 bytes

msg = 12 bytes

msg = chunck + 32 bytes + 24 bytes

 

uint8_t key        [32];    // Random, secret session key  

uint8_t nonce      [24];    // Use only once per key       

uint8_t plain_text [12] = "Lorem ipsum"; // Secret message 

uint8_t mac        [16];    // Message authentication code 

uint8_t cipher_text[12];              // Encrypted message 

 

*/


/* called once per number_measurements */

void prepare_inputs(dudect_config_t *c, uint8_t *input_data, uint8_t *classes) {

    // randombytes(input_data, c->number_measurements * c->chunk_size);

    // monocypher RNG impl:

    getrandom(input_data, c->number_measurements * c->chunk_size, 0);

 

    for (size_t i = 0; i < c->number_measurements; i++) {

    /* it is important to randomize the class sequence */

    classes[i] = randombit();

    if (classes[i] == 0) {

        memset(input_data + (size_t)i * c->chunk_size, 0x00, c->chunk_size);

    } else {

        // leave random

    }

    }

}



int main(){

}