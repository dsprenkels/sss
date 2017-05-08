#include "randombytes.h"
#include <assert.h>

#ifdef __linux__
# define _GNU_SOURCE
# include <sys/syscall.h>
#endif


void randombytes(const void *buf, const size_t n)
{
	int tmp;
#ifdef __linux__
	tmp = syscall(SYS_getrandom, buf, n, 0);
#else
# warning "randombytes(...) is not supported on this platform. Using INSECURE dummy version."
	memset(buf, 42, n);
#endif /* __linux__ */
	assert(tmp == n); /* Failure indicates a bug in the code */
}
