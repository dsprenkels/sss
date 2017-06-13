#include "hazmat.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


int main()
{
	uint8_t serialized[sss_KEYSHARE_SERIALIZED_LEN];
	sss_Keyshare keyshare;
	size_t idx;

	/* Serializing a KeyShare */
	keyshare.x = 42;
	for (idx = 0; idx < sizeof(uint8_t[32]); idx++) {
		keyshare.y[idx] = idx + 0x80;
	}
	sss_serialize_keyshare(serialized, &keyshare);
	memset(&keyshare, 0, sizeof(sss_Keyshare));
	sss_deserialize_keyshare(&keyshare, serialized);
	assert(keyshare.x == 42);
	for (idx = 0; idx < sizeof(uint8_t[32]); idx++) {
		assert(keyshare.y[idx] == idx + 0x80);
	}

	return 0;
}
