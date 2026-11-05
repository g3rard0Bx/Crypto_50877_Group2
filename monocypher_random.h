#ifndef MONOCYPHER_RANDOM_H
#define MONOCYPHER_RANDOM_H

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/random.h>

// Added: fill the entire buffer; retry interrupted calls and handle short reads.
// Return failure instead of allowing an incompletely initialized key or nonce.
static inline int monocypher_random(uint8_t *out, size_t size)
{
    while (size != 0) {
        ssize_t count = getrandom(out, size, 0);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (count == 0) {
            errno = EIO;
            return -1;
        }
        out += (size_t)count;
        size -= (size_t)count;
    }
    return 0;
}
#endif
