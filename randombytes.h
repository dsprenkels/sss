#ifndef sss_RANDOMBYTES_H
#define sss_RANDOMBYTES_H

#include <sys/syscall.h>
#include <unistd.h>


/*
 * Write `n` bytes of high quality random bytes to `buf`
 */
void randombytes(void *buf, size_t n);


#endif /* sss_RANDOMBYTES_H */
