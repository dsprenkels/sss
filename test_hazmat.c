#include "hazmat.c"
#include "randombytes.c"
#include <assert.h>
#include <string.h>

static void test_gf256_mul()
{
	assert(gf256_mul(0x00, 0x00) == 0x00);
	assert(gf256_mul(0x01, 0x00) == 0x00);
	assert(gf256_mul(0x00, 0x01) == 0x00);
	assert(gf256_mul(0x01, 0x01) == 0x01);
	assert(gf256_mul(0x88, 0x88) == 0xda);
	assert(gf256_mul(0xff, 0xff) == 0x13);
}

static void test_gf256_inv()
{
	size_t idx;
	uint8_t tmp;

	assert(gf256_inv(0) == 0);
	for (idx = 1; idx < 256; idx++) {
		tmp = idx;
		assert(gf256_mul(gf256_inv(tmp), tmp) == 1);
	}
}


static void test_key_shares()
{
	uint8_t key[32], restored[32];
	SSS_Keyshare key_shares[256];
	size_t idx;

	for (idx = 0; idx < 32; idx++) {
		key[idx] = idx;
	}

	SSS_create_keyshares(key_shares, key, 1, 1);
	SSS_combine_keyshares(restored, key_shares, 1);
	assert(memcmp(key, restored, 32) == 0);

	SSS_create_keyshares(key_shares, key, 3, 2);
	SSS_combine_keyshares(restored, &key_shares[1], 2);
	assert(memcmp(key, restored, 32) == 0);

	SSS_create_keyshares(key_shares, key, 255, 127);
	SSS_combine_keyshares(restored, &key_shares[128], 127);
	assert(memcmp(key, restored, 32) == 0);

	SSS_create_keyshares(key_shares, key, 255, 255);
	SSS_combine_keyshares(restored, key_shares, 255);
	assert(memcmp(key, restored, 32) == 0);
}


int main()
{
	test_gf256_mul();
	test_gf256_inv();
	test_key_shares();
	return 0;
}
