#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <cstdint>
#include <string.h>
#include <valgrind/memcheck.h>
// #include <windows.h>
// #include <bcrypt.h>
#include <sys/random.h>
#include <unistd.h>
#include <vector>


#include "monocypher.h"
#include <assert.h>

#define CONTEXT "Example"
#define MESSAGE "test"
#define MESSAGE_LEN (sizeof(MESSAGE))
#define CIPHERTEXT_LEN (hydro_secretbox_HEADERBYTES + MESSAGE_LEN)

#pragma comment(lib, "bcrypt.lib")

using namespace std;


// monocypher
// implementations: dudect and valgrind

void valgrind_impl(){
    
    
    uint8_t key        [32];    /* Random, secret session key  */
    uint8_t nonce      [24];    /* Use only once per key       */
    uint8_t plain_text [12] = "Lorem ipsum"; /* Secret message */
    uint8_t mac        [16];    /* Message authentication code */
    uint8_t cipher_text[12];              /* Encrypted message */
    uint8_t decrypted[sizeof(plain_text)];
    // BCryptGenRandom(key, 32, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    // BCryptGenRandom(nonce, 24, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    getrandom(key, sizeof(key), 0);
    getrandom(nonce, sizeof(nonce), 0);
    

    
    VALGRIND_MAKE_MEM_UNDEFINED(key, sizeof(key));
    VALGRIND_MAKE_MEM_UNDEFINED(plain_text, sizeof(plain_text));
    
    
    
    crypto_aead_lock(cipher_text, mac,
                    key, nonce,
                    NULL, 0,
                    plain_text, sizeof(plain_text));
    

    // 
    VALGRIND_MAKE_MEM_DEFINED(cipher_text, sizeof(cipher_text));
    VALGRIND_MAKE_MEM_DEFINED(mac, sizeof(mac));
    


    VALGRIND_MAKE_MEM_UNDEFINED(key, sizeof(key));
    VALGRIND_MAKE_MEM_UNDEFINED(cipher_text,sizeof(cipher_text));
    VALGRIND_MAKE_MEM_UNDEFINED(mac, sizeof(mac));
    
    // decryption:
    int status = crypto_aead_unlock(decrypted, mac,
                    key, nonce,
                    NULL, 0,
                    cipher_text, sizeof(cipher_text));
    
    VALGRIND_MAKE_MEM_DEFINED(&status, sizeof(status));
    assert(status==0);
    
    VALGRIND_MAKE_MEM_DEFINED(decrypted, sizeof(cipher_text));


    VALGRIND_MAKE_MEM_DEFINED(key, sizeof(key));
    crypto_wipe(key, sizeof(key));
    crypto_wipe(decrypted, sizeof(decrypted));
    


}



int main(){

    
    

    return;
}
