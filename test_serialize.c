#include "serialize.h"
#include "hazmat.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


int main()
{
	uint8_t data[sss_MLEN] = { 42 }, restored[sss_MLEN];
	uint8_t serialized[sss_SHARE_SERIALIZED_LEN];
	sss_Keyshare keyshare;
	sss_Share share;
	size_t idx;
	int tmp;

	/* Serializing a KeyShare */
	keyshare.x = 42;
	for (idx = 0; idx < sizeof(uint8_t[32]); idx++) {
		keyshare.y[idx] = idx + 0x80;
	}
	sss_serialize_keyshare(serialized, &keyshare);
	memset(&keyshare, 0, sizeof(sss_Keyshare));
	sss_unserialize_keyshare(&keyshare, serialized);
	assert(keyshare.x == 42);
	for (idx = 0; idx < sizeof(uint8_t[32]); idx++) {
		assert(keyshare.y[idx] == idx + 0x80);
	}

	/* Serializing a normal Share */
	share.keyshare = keyshare;
	for (idx = 0; idx < sss_CLEN; idx++) {
		share.c[idx] = idx + 0x80;
	}
	sss_serialize_share(serialized, &share);
	memset(&share, 0, sizeof(sss_Share));
	sss_unserialize_share(&share, serialized);
	assert(share.keyshare.x == 42);
	for (idx = 0; idx < sizeof(uint8_t[32]); idx++) {
		assert(share.keyshare.y[idx] == idx + 0x80);
	}
	for (idx = 0; idx < sss_CLEN; idx++) {
		assert(share.c[idx] == idx + 0x80);
	}

	/* Normal operation with one share */
	sss_create_shares(&share, data, 1, 1);
	sss_serialize_share(serialized, &share);
	memset(&share, 0, sizeof(sss_Share));
	sss_unserialize_share(&share, serialized);
	tmp = sss_combine_shares(restored, &share, 1);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);

	return 0;
}
