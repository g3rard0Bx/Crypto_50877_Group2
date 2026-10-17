#include <stdio.h>
#include <valgrind/memcheck.h>
#include "hydrogen.h"

#define CONTEXT "Example"
#define MESSAGE "test"
#define MESSAGE_LEN (sizeof(MESSAGE))
#define CIPHERTEXT_LEN (hydro_secretbox_HEADERBYTES + MESSAGE_LEN)

using namespace std;


/*
poisoning in valgrind:

Poisoning only flags an error if the code handles the secret data dangerously

Unpoisoning:
tells valgrind that secret data is safe and to treat as a normal regular memory
unpoisoning ensures that we do not have any false flags/errors and prevents us from thinking
that the security was non-constant time.

what it looks out for:
secret-dependent branching (different amount of time to execute (non-constant time))
secret-dependent cache access (cache attacks)

some examples of how to utilize the libhydrogen functions/arguments:
https://github.com/jedisct1/libhydrogen/wiki/Secret-key-encryption

*/
// utilizing standard valgrind
void valgrind_encryption_phase (uint8_t secret_key[hydro_secretbox_KEYBYTES]){

    uint8_t msg[] = MESSAGE;
    
    size_t msg_len = MESSAGE_LEN;
    
    uint8_t ciphertext[CIPHERTEXT_LEN];

    
    // pass secret key and exact size in bytes req for a symmetric secret encryption key 
    VALGRIND_MAKE_MEM_UNDEFINED(secret_key, hydro_secretbox_KEYBYTES);
    VALGRIND_MAKE_MEM_UNDEFINED(msg, msg_len);

    // libhydrogen encryption:
    int res = hydro_secretbox_encrypt(ciphertext,msg,msg_len,0,CONTEXT,secret_key);
    if (res != 0) {
        fprintf(stderr, "Invalid... encryption failed");
    }
    
    VALGRIND_MAKE_MEM_DEFINED(ciphertext, sizeof(ciphertext));
    VALGRIND_MAKE_MEM_DEFINED(secret_key, hydro_secretbox_KEYBYTES);
    VALGRIND_MAKE_MEM_DEFINED(msg, msg_len);
    
    uint8_t decrypted[MESSAGE_LEN];

    VALGRIND_MAKE_MEM_UNDEFINED(secret_key, hydro_secretbox_KEYBYTES);
    VALGRIND_MAKE_MEM_UNDEFINED(ciphertext, CIPHERTEXT_LEN);

    
    // decryption:
    int res = hydro_secretbox_decrypt(decrypted, ciphertext, CIPHERTEXT_LEN, 0, CONTEXT, secret_key);
    if (res != 0) {
        fprintf(stderr, "Invalid... decryption failed");
    }

    VALGRIND_MAKE_MEM_DEFINED(secret_key,hydro_secretbox_KEYBYTES);
    VALGRIND_MAKE_MEM_DEFINED(ciphertext, CIPHERTEXT_LEN);
    VALGRIND_MAKE_MEM_DEFINED(decrypted, msg_len);


}



int main (){
    // set up:
    if (hydro_init() != 0) {
        abort();
    }

    uint8_t key[hydro_secretbox_KEYBYTES];
    // generates suitable key
    hydro_secretbox_keygen(key);
    
    // valgrind_encryption_phase(key);

    return 0;
}