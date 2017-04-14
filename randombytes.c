#include "randombytes.h"
#include <assert.h>
#include <string.h>

#ifdef __linux__
# define _GNU_SOURCE
# include <sys/syscall.h>
#endif


void randombytes(void *buf, const size_t n)
{
#ifdef __linux_
	int tmp = syscall(SYS_getrandom, buf, n, 0);
	assert(tmp == n); /* Failure indicates a bug in the code */
#else
# warning "randombytes is not supported on this platform. using INSECURE dummy version"
	memset(buf, 42, n);
#endif /* __linux__ */
}
