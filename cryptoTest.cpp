#include <stdio.h>
#include <valgrind/memcheck.h>


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

*/
// utilizing standard valgrind
void crypto_standard_valgrind (unsigned char *secret_key, size_t len){

}

int main (){

    return 0;
}