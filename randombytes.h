#ifndef SSS_RANDOMBYTES_H
#define SSS_RANDOMBYTES_H

#include <sys/syscall.h>
#include <unistd.h>


/*
 * Write `n` bytes of high quality random bytes to `buf`
 */
void randombytes(const void *buf, size_t n);


#endif /* SSS_RANDOMBYTES_H */
