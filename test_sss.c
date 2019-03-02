#include "sss.h"
#include <assert.h>
#include <string.h>

static void test_constant_length(void)
{
	unsigned char data[sss_MLEN] = { 42 }, restored[sss_MLEN];
	sss_Share shares[256];
	int tmp;

	/* Normal operation */
	sss_create_shares(shares, data, 1, 1);
	tmp = sss_combine_shares(restored, (const sss_Share*) shares, 1);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	/* A lot of shares */
	sss_create_shares(shares, data, 255, 255);
	tmp = sss_combine_shares(restored, (const sss_Share*) shares, 255);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	/* Not enough shares to restore secret */
	sss_create_shares(shares, data, 100, 100);
	tmp = sss_combine_shares(restored, (const sss_Share*) shares, 99);
	assert(tmp == -1);

	/* Too many secrets should also restore the secret */
	sss_create_shares(shares, data, 200, 100);
	tmp = sss_combine_shares(restored, (const sss_Share*) shares, 200);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);
}

static void test_variable_length(void)
{
	unsigned char data[sss_VARIABLE_MAX_MLEN] = { 42 }, restored[sss_VARIABLE_MAX_MLEN];
	uint8_t shares[sss_VARIABLE_MAX_SHARE_LEN * 256];
	int tmp;

	/* Normal operation */
	sss_create_shares_varlen(shares, data, sss_MLEN, 2, 2);
	tmp = sss_combine_shares_varlen(restored, shares, sss_SHARE_LEN, 2);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	/* A lot of shares */
	sss_create_shares_varlen(shares, data, sss_MLEN, 255, 255);
	tmp = sss_combine_shares_varlen(restored, shares, sss_SHARE_LEN, 255);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	/* Not enough shares to restore secret */
	sss_create_shares_varlen(shares, data, sss_MLEN, 100, 100);
	tmp = sss_combine_shares_varlen(restored, shares, sss_SHARE_LEN, 99);
	assert(tmp == -1);

	/* Too many secrets should also restore the secret */
	sss_create_shares_varlen(shares, data, sss_MLEN, 200, 100);
	tmp = sss_combine_shares_varlen(restored, shares, sss_SHARE_LEN, 200);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	/* Max-length secret */
	sss_create_shares_varlen(shares, data, sss_VARIABLE_MAX_MLEN, 255, 255);
	tmp = sss_combine_shares_varlen(restored, shares, sss_VARIABLE_MAX_MLEN + 49, 255);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_VARIABLE_MAX_MLEN) == 0);

	/* Zero-length secret */
	sss_create_shares_varlen(shares, data, 0, 255, 255);
	tmp = sss_combine_shares_varlen(restored, shares, 49, 255);
	assert(tmp == 0);
}

int main(void) {
	test_constant_length();
	test_variable_length();
	return 0;
}
