#include "sss.h"
#include <assert.h>
#include <string.h>

int main()
{
	uint8_t data[sss_MLEN] = { 42 }, restored[sss_MLEN];
	uint8_t key[32] = { 0 };
	sss_Share shares[256];
	int tmp;

	/* Normal operation */
	sss_create_shares(shares, data, 1, 1, key);
	tmp = sss_combine_shares(restored, shares, 1);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	/* A lot of shares */
	sss_create_shares(shares, data, 255, 255, key);
	tmp = sss_combine_shares(restored, shares, 255);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	/* Not enough shares to restore secret */
	sss_create_shares(shares, data, 100, 100, key);
	tmp = sss_combine_shares(restored, shares, 99);
	assert(tmp == -1);

	/* Too many secrets should also restore the secret */
	sss_create_shares(shares, data, 200, 100, key);
	tmp = sss_combine_shares(restored, shares, 200);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	/* Weird inputs */
	sss_create_shares(shares, data, 0, 0, key);
	tmp = sss_combine_shares(restored, shares, 0);
	assert(tmp == -1);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	return 0;
}
