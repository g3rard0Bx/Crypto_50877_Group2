// Purpose: This test is meant to demonstrate the functionality of the
// Valgrind functions. If a conditional is based on an undefined value,
// then Valgrind will detect an error.

#include <stdio.h>
#include <stdint.h>
#include <valgrind/memcheck.h>

int main(void)
{
    uint8_t secret = 1;

    // Mark secret as undefined
    VALGRIND_MAKE_MEM_UNDEFINED(&secret, sizeof(secret));

    // Make a conditional depend on the undefined secret
    if (secret == 1) {
        printf("Secret is one.\n");
    } else {
        printf("Secret is not one.\n");
    }

    // Re-define the secret before exiting
    VALGRIND_MAKE_MEM_DEFINED(&secret, sizeof(secret));

    return 0;
}