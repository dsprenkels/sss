#include "hazmat.h"
#include <assert.h>
#include <string.h>

static void test_key_shares(void)
{
	uint8_t key[32], restored[32];
	sss_Keyshare key_shares[256];
	size_t idx;

	for (idx = 0; idx < 32; idx++)
	{
		key[idx] = idx;
	}

	sss_create_keyshares(key_shares, key, 1, 1);
	sss_combine_keyshares(restored, (const sss_Keyshare *)key_shares, 1);
	assert(memcmp(key, restored, 32) == 0);

	sss_create_keyshares(key_shares, key, 3, 2);
	sss_combine_keyshares(restored, (const sss_Keyshare *)key_shares[1], 2);
	assert(memcmp(key, restored, 32) == 0);

	sss_create_keyshares(key_shares, key, 255, 127);
	sss_combine_keyshares(restored, (const sss_Keyshare *)key_shares[128], 127);
	assert(memcmp(key, restored, 32) == 0);

	sss_create_keyshares(key_shares, key, 255, 255);
	sss_combine_keyshares(restored, (const sss_Keyshare *)key_shares, 255);
	assert(memcmp(key, restored, 32) == 0);
}

static void test_add_Keyshares()
{
	uint8_t key1[32], key2[32], key1_add_key2[32], restored[32];
	sss_Keyshare key_shares1[10], key_shares2[10];
	size_t idx;
	for (idx = 0; idx < 32; idx++)
	{
		key1[idx] = idx;
		key2[idx] = idx * 2;
		key1_add_key2[idx] = key1[idx] ^ key2[idx];
	}

	sss_create_keyshares(key_shares1, key1, 10, 5);
	// sss_combine_keyshares(restored, (const sss_Keyshare *)key_shares1, 5);
	sss_create_keyshares(key_shares2, key2, 10, 5);
	add_Keyshares(key_shares1, key_shares2, 10);
	sss_combine_keyshares(restored, (const sss_Keyshare *)key_shares1, 5);
	// 输出key1_add_key2
	printf("key1_add_key2:\n");
	for (idx = 0; idx < 32; idx++)
	{
		printf("%d ", key1_add_key2[idx]);
	}
	printf("\n");
	// 输出restored
	printf("restored:\n");
	for (idx = 0; idx < 32; idx++)
	{
		printf("%d ", restored[idx]);
	}
	printf("\n");
	assert(memcmp(key1_add_key2, restored, 32) == 0);
}

int main(void)
{
	// test_key_shares();
	test_add_Keyshares();
	return 0;
}
