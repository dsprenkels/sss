#include "sss.h"
#include <assert.h>
#include <string.h>

int main()
{
	unsigned char data[SSS_MLEN] = { 42 }, restored[SSS_MLEN];
	SSS_Share shares[256];
	int tmp;

	/* Normal operation */
	SSS_create_shares(shares, data, 1, 1);
	tmp = SSS_combine_shares(restored, shares, 1);
	assert(tmp == 0);
	assert(memcmp(restored, data, SSS_MLEN) == 0);

	/* A lot of shares */
	SSS_create_shares(shares, data, 255, 255);
	tmp = SSS_combine_shares(restored, shares, 255);
	assert(tmp == 0);
	assert(memcmp(restored, data, SSS_MLEN) == 0);

	/* Not enough shares to restore secret */
	SSS_create_shares(shares, data, 100, 100);
	tmp = SSS_combine_shares(restored, shares, 99);
	assert(tmp == -1);

	/* Too many secrets should also restore the secret */
	SSS_create_shares(shares, data, 200, 100);
	tmp = SSS_combine_shares(restored, shares, 200);
	assert(tmp == 0);
	assert(memcmp(restored, data, SSS_MLEN) == 0);

	/* Weird inputs */
	SSS_create_shares(shares, data, 0, 0);
	tmp = SSS_combine_shares(restored, shares, 0);
	assert(tmp == -1);
	assert(memcmp(restored, data, SSS_MLEN) == 0);

	return 0;
}
