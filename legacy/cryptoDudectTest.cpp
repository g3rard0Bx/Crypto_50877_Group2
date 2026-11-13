#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hydrogen.h"

/*
    required functions from dudect:
        -do_one_computation()
        -prepare_inputs()
        -dudect_main()

*/
#define DUDECT_IMPLEMENTATION
#include "dudect.h"


#define CONTEXT "Example"
#define MESSAGE "test"
#define MESSAGE_LEN (sizeof(MESSAGE))
#define CIPHERTEXT_LEN (hydro_secretbox_HEADERBYTES + MESSAGE_LEN)


using namespace std;

/* this will be called over and over */
uint8_t do_one_computation(uint8_t *data) {
  /* simulate totally bogus MAC check in non-constant time */
//   return check_tag(data, secret, SECRET_LEN_BYTES);
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



int main (){
    // set up:
    if (hydro_init() != 0) {
        abort();
    }

    uint8_t key[hydro_secretbox_KEYBYTES];
    // generates suitable key
    hydro_secretbox_keygen(key);

    dudect_config_t config = {
      // .chunk_size = SECRET_LEN_BYTES,
      #ifdef MEASUREMENTS_PER_CHUNK
      .number_measurements = MEASUREMENTS_PER_CHUNK,
      #else
      .number_measurements = 500,
      #endif
    };
    dudect_ctx_t ctx;

    dudect_init(&ctx, &config);



    return 0;
}
