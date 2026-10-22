#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <cstdint>
#include <string.h>
#include <valgrind/memcheck.h>

#include "monocypher.h"

#define CONTEXT "Example"
#define MESSAGE "test"
#define MESSAGE_LEN (sizeof(MESSAGE))
#define CIPHERTEXT_LEN (hydro_secretbox_HEADERBYTES + MESSAGE_LEN)


using namespace std;


// monocypher
// implementations: dudect and valgrind

void valgrind_impl(){
    
    
    
    uint8_t key        [32];    /* Random, secret session key  */
    uint8_t nonce      [24];    /* Use only once per key       */
    uint8_t plain_text [12] = "Lorem ipsum"; /* Secret message */
    uint8_t mac        [16];    /* Message authentication code */
    uint8_t cipher_text[12];              /* Encrypted message */
    arc4random_buf(key,   32);
    arc4random_buf(nonce, 24);
    
    VALGRIND_MAKE_MEM_UNDEFINED(key, sizeof(key));
    VALGRIND_MAKE_MEM_UNDEFINED(plain_text, sizeof(plain_text));
    
    
    
    crypto_aead_lock(cipher_text, mac,
                    key, nonce,
                    NULL, 0,
                    plain_text, sizeof(plain_text));
    /* Wipe secrets if they are no longer needed */
    crypto_wipe(plain_text, 12);
    crypto_wipe(key, 32);
    /* Transmit cipher_text, nonce, and mac over the network,
    * store them in a file, etc.
    */
    
    VALGRIND_MAKE_MEM_DEFINED(key, sizeof(key));
    VALGRIND_MAKE_MEM_DEFINED(plain_text, sizeof(plain_text));
    VALGRIND_MAKE_MEM_DEFINED(cipher_text,sizeof(cipher_text));
    VALGRIND_MAKE_MEM_DEFINED(mac, sizeof(mac));
    



}


// // Encryption:


int main(){

    
    

    return;
}
