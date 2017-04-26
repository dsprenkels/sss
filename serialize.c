#include "serialize.h"
#include <string.h>


void sss_serialize_keyshare(uint8_t *out, const sss_Keyshare *keyshare)
{
	out[0] = keyshare->x;
	memcpy(&out[1], &keyshare->y, sizeof(uint8_t[32]));
}


void sss_unserialize_keyshare(sss_Keyshare *keyshare, const uint8_t *in)
{
	keyshare->x = in[0];
	memcpy(&keyshare->y, &in[1], sizeof(uint8_t[32]));
}


void sss_serialize_share(uint8_t *out, const sss_Share *share)
{
	sss_serialize_keyshare(out, &share->keyshare);
	memcpy(&out[sss_KEYSHARE_SERIALIZED_LEN], &share->c, sss_CLEN);
}


void sss_unserialize_share(sss_Share *share, const uint8_t *in)
{
	sss_unserialize_keyshare(&share->keyshare, in);
	memcpy(&share->c, &in[sss_KEYSHARE_SERIALIZED_LEN], sss_CLEN);
}
