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




static const uint8_t plain_text [TEXT_SIZE] = "Lorem ipsum";

// dudect

/*
    functions to implement:
    do_one_computation()
    prepare_inputs()
    call dudect_main() from main function
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


// void encryption_monocypher(){
//   uint8_t key        [32];    /* Random, secret session key  */
//   uint8_t nonce      [24];    /* Use only once per key       */
//   uint8_t plain_text [12] = "Lorem ipsum"; /* Secret message */
//   uint8_t mac        [16];    /* Message authentication code */
//   uint8_t cipher_text[12];              /* Encrypted message */
//   arc4random_buf(key,   32);
//   arc4random_buf(nonce, 24);
//   crypto_aead_lock(cipher_text, mac,
//                   key, nonce,
//                   NULL, 0,
//                   plain_text, sizeof(plain_text));
//   /* Wipe secrets if they are no longer needed */
//   crypto_wipe(plain_text, 12);
//   crypto_wipe(key, 32);
//   /* Transmit cipher_text, nonce, and mac over the network,
//   * store them in a file, etc.
//   */
// }

// // encryption and decryption will be separated into their own file
// void decryption_monocypher(){
//   uint8_t key        [32];    /* Random, secret session key  */
//   uint8_t nonce      [24];    /* Use only once per key       */
//   uint8_t plain_text [12] = "Lorem ipsum"; /* Secret message */
//   uint8_t mac        [16];    /* Message authentication code */
//   uint8_t cipher_text[12];              /* Encrypted message */
//   arc4random_buf(key,   32);
//   arc4random_buf(nonce, 24);
//   crypto_aead_lock(cipher_text, mac,
//                   key, nonce,
//                   NULL, 0,
//                   plain_text, sizeof(plain_text));
//   /* Wipe secrets if they are no longer needed */
//   crypto_wipe(plain_text, 12);
//   crypto_wipe(key, 32);
//   /* Transmit cipher_text, nonce, and mac over the network,
//   * store them in a file, etc.
//   */
// }


/* this will be called over and over */
// we want encryption or decryption
uint8_t do_one_computation(uint8_t *data) {
  /* simulate totally bogus MAC check in non-constant time */
  // return check_tag(data, secret, SECRET_LEN_BYTES);

  uint8_t *key = data;    /* Random, secret session key  */
  uint8_t *nonce = data + SECRET_KEY_SIZE;    

  uint8_t mac [MAC_SIZE];   
  uint8_t cipher_text[TEXT_SIZE];

  // arc4random on windows
  // arc4random_buf(key,   32);
  // arc4random_buf(nonce, 24);

  // using randombytes function to work across different systems 
  randombytes(key, 32);
  randombytes(nonce, 24);
  
  // encryption:
  crypto_aead_lock(cipher_text, mac,
                  key, nonce,
                  NULL, 0,
                  plain_text, sizeof(plain_text));
  
  
  
  return memcmp(plain_text, cipher_text, TEXT_SIZE);
}


/* called once per number_measurements */

void prepare_inputs(dudect_config_t *c, uint8_t *input_data, uint8_t *classes) {
    // c->chunk_size = TEXT_SIZE;

    randombytes(input_data, c->number_measurements * c->chunk_size);

    // // monocypher RNG impl:
    // getrandom(input_data, c->number_measurements * c->chunk_size, 0);

    for (size_t i = 0; i < c->number_measurements; i++) {
      /* it is important to randomize the class sequence */
      classes[i] = randombit();

      if (classes[i] == 0) {
        // do not alter nonce (gets generated randomly)
        //c->chunk_size ==> SECRET_KEY_LENGTH
        // isolates secret key while testing for leakage
        // class 0 = random, class 1 = random which isn't what we want
        //instead class 0 = fixed, class 1 = random based on secret key  
          memset(input_data + (size_t)i * c->chunk_size, 0x00, SECRET_KEY_SIZE);
      } else {
          // leave random
      }

    }

}


int run_test(void) {

  dudect_config_t config = {
      // .chunk_size = SECRET_LEN_BYTES,
      .number_measurements = 500,
  };

  dudect_ctx_t ctx;
  dudect_init(&ctx, &config);

  /*
  From dudect repo:
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

  if(state == DUDECT_LEAKAGE_FOUND){
    printf("--------------------------------");
    printf("\n[!] Timing Leakage Detected!\n");
    printf("--------------------------------");
  }

  return (int)state;
}


int main(){
  run_test();
  return;
}