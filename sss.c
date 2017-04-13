#include "randombytes.h"
#include "tweetnacl.h"
#include "sss.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * These assertions may be considered overkill, but would if the tweetnacl API
 * ever change we *really* want to prevent buffer overflow vulnerabilities.
 */
#if crypto_secretbox_KEYBYTES != 32
# error "crypto_secretbox_KEYBYTES size is invalid"
#endif


/*
 * Nonce for the `crypto_secretbox` authenticated encryption.
 * The nonce is constant (zero), because we are using an ephemeral key.
 */
static const unsigned char nonce[crypto_secretbox_NONCEBYTES] = { 0 };


void SSS_create_shares(SSS_Share *out, const unsigned char *data,
	               uint8_t n, uint8_t k)
{
	const unsigned char key[crypto_secretbox_KEYBYTES];
	unsigned char m[crypto_secretbox_ZEROBYTES + SSS_MLEN] = { 0 };
	unsigned long long mlen = sizeof(m); /* length includes zero-bytes */
	unsigned char c[mlen];
	int tmp;
	SSS_Keyshare keyshares[n];
	size_t idx;

	/* Generate a random key */
	randombytes(key, crypto_secretbox_KEYBYTES);

	/* AEAD encrypt the data with the key */
	memcpy(&m[crypto_secretbox_ZEROBYTES], data, SSS_MLEN);
	tmp = crypto_secretbox(c, m, mlen, nonce, key);
	assert(tmp == 0); /* should always happen */

	/* Generate KeyShares */
	SSS_create_keyshares(keyshares, key, n, k);

	/* Build regular shares */
	for (idx = 0; idx < mlen; idx++) {
		out[idx].keyshare = keyshares[idx];
		memcpy(out[idx].c, &c[crypto_secretbox_BOXZEROBYTES], SSS_MLEN);
	}
}


int SSS_combine_shares(uint8_t *data, const SSS_Share *shares, uint8_t k)
{
	/* int crypto_secretbox_open(u8 *m,const u8 *c,u64 mlen,const u8 *nonce,const 	u8 *key) */

	unsigned char key[crypto_secretbox_KEYBYTES];
	unsigned char c[crypto_secretbox_BOXZEROBYTES + SSS_CLEN] = { 0 };
	unsigned long long clen = sizeof(c);
	unsigned char m[clen];
	SSS_Keyshare keyshares[k];
	size_t idx;
	int ret = 0;

	/* Check if all ciphertexts are the same */
	if (k < 1) return -1;
	for (idx = 1; idx < k; idx++) {
		if (memcmp(shares[0].c, shares[idx].c, SSS_CLEN) != 0) {
			return -1;
		}
	}

	/* Restore the key */
	for (idx = 0; idx < k; idx++) {
		keyshares[idx] = shares[idx].keyshare;
	}
	SSS_combine_keyshares(key, keyshares, k);

	/* Decrypt the ciphertext */
	memcpy(&c[crypto_secretbox_BOXZEROBYTES], shares[0].c, SSS_CLEN);
	ret |= crypto_secretbox_open(m, c, clen, nonce, key);
	memcpy(data, &m[crypto_secretbox_ZEROBYTES], SSS_MLEN);

	return ret;
}