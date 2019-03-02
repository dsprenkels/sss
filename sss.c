/*
 * AEAD wrapper around the Secret shared data
 *
 * Author: Daan Sprenkels <hello@dsprenkels.com>
 *
 * This module implements a AEAD wrapper around some secret shared data,
 * allowing the data to be in any format. (Directly secret-sharing requires the
 * message to be picked uniformly in the message space.)
 *
 * The NaCl cryptographic library is used for the encryption. The encryption
 * scheme that is used for wrapping the message is salsa20/poly1305. Because
 * we are using an ephemeral key, we are using a zero'd nonce.
 */


#include "randombytes.h"
#include "tweetnacl.h"
#include "sss.h"
#include "tweetnacl.h"
#include <assert.h>
#include <string.h>


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


/*
 * Return a mutable pointer to the ciphertext part of this Share
 */
static uint8_t* get_ciphertext(sss_Share *share)
{
	return &((uint8_t*) share)[sss_KEYSHARE_LEN];
}


/*
 * Return a mutable pointer to the Keyshare part of this Share
 */
static sss_Keyshare* get_keyshare(sss_Share *share)
{
	return (sss_Keyshare*) &share[0];
}


/*
 * Return a const pointer to the ciphertext part of this Share
 */
static const uint8_t* get_ciphertext_const(const sss_Share *share)
{
	return &((const uint8_t*) share)[sss_KEYSHARE_LEN];
}


/*
 * Return a mutable pointer to the ciphertext part of this Share
 */
static uint8_t* get_varlen_ciphertext(uint8_t *share)
{
	return &share[sss_KEYSHARE_LEN];
}


/*
 * Return a mutable pointer to the Keyshare part of this Share
 */
static sss_Keyshare* get_varlen_keyshare(uint8_t *share)
{
	return (sss_Keyshare*) &share[0];
}


/*
 * Return a const pointer to the ciphertext part of this Share
 */
static const uint8_t* get_varlen_ciphertext_const(const uint8_t *share)
{
	return &share[sss_KEYSHARE_LEN];
}


/*
 * Create `n` shares with theshold `k` and write them to `out`
 */
void sss_create_shares(sss_Share *out, const unsigned char *data,
                       uint8_t n, uint8_t k)
{
	unsigned char key[32];
	unsigned char m[crypto_secretbox_ZEROBYTES + sss_MLEN] = { 0 };
	unsigned long long mlen = sizeof(m); /* length includes zero-bytes */
	unsigned char c[crypto_secretbox_BOXZEROBYTES + sss_CLEN];
	int tmp;
	sss_Keyshare keyshares[n];
	size_t idx;

	/* Generate a random encryption key */
	randombytes(key, sizeof(key));

	/* AEAD encrypt the data with the key */
	memcpy(&m[crypto_secretbox_ZEROBYTES], data, sss_MLEN);
	tmp = crypto_secretbox(c, m, mlen, nonce, key);
	assert(tmp == 0); /* should always happen */

	/* Generate KeyShares */
	sss_create_keyshares(keyshares, key, n, k);

	/* Build regular shares */
	for (idx = 0; idx < n; idx++) {
		memcpy(get_keyshare((sss_Share*) &out[idx]), &keyshares[idx][0],
		       sss_KEYSHARE_LEN);
		memcpy(get_ciphertext((sss_Share*) &out[idx]),
		       &c[crypto_secretbox_BOXZEROBYTES], sss_CLEN);
	}
}

/*
 * Create `n` shares with theshold `k` and write them to `out`, where
 * the `data` is `len` bytes long, not more than 4096 bytes.
 */
int sss_create_shares_varlen(uint8_t *out, const uint8_t *data,
	                     const size_t data_len, uint8_t n, uint8_t k)
{
	unsigned char key[32];
	unsigned char m[crypto_secretbox_ZEROBYTES + sss_VARIABLE_MAX_MLEN] = { 0 };
	unsigned char c[crypto_secretbox_BOXZEROBYTES + sss_VARIABLE_MAX_CLEN];
	// `mlen` explicitly includes the zero-bytes
	unsigned long long mlen = crypto_secretbox_ZEROBYTES + data_len;
	int tmp;
	sss_Keyshare keyshares[n];
	size_t idx, offset;

	if (data_len > 4096) {
		return -1;
	}

	/* Generate a random encryption key */
	randombytes(key, sizeof(key));

	/* AEAD encrypt the data with the key */
	memcpy(&m[crypto_secretbox_ZEROBYTES], data, data_len);
	tmp = crypto_secretbox(c, m, mlen, nonce, key);
	assert(tmp == 0); /* should always happen */

	/* Generate KeyShares */
	sss_create_keyshares(keyshares, key, n, k);

	/* Build regular shares */
	for (idx = 0; idx < n; idx++) {
		offset = idx * (sss_KEYSHARE_LEN + data_len + 16);
		memcpy(get_varlen_keyshare(&out[offset]), &keyshares[idx][0],
		       sss_KEYSHARE_LEN);
		memcpy(get_varlen_ciphertext(&out[offset]),
			&c[crypto_secretbox_BOXZEROBYTES], data_len + 16);
	}
	return 0;
}


/*
 * Combine `k` shares pointed to by `shares` and write the result to `data`
 *
 * This function returns -1 if any of the shares were corrupted or if the number
 * of shares was too low. It is not possible to detect which of these errors
 * did occur.
 */
int sss_combine_shares(uint8_t *data, const sss_Share *shares, uint8_t k)
{
	unsigned char key[crypto_secretbox_KEYBYTES];
	unsigned char c[crypto_secretbox_BOXZEROBYTES + sss_CLEN] = { 0 };
	unsigned long long clen = sizeof(c);
	unsigned char m[crypto_secretbox_ZEROBYTES + sss_MLEN];
	sss_Keyshare keyshares[k];
	size_t idx;
	int ret = 0;

	/* Check if all ciphertexts are the same */
	if (k < 1) return -1;
	for (idx = 1; idx < k; idx++) {
		if (memcmp(get_ciphertext_const(&shares[0]),
		           get_ciphertext_const(&shares[idx]), sss_CLEN) != 0) {
			return -1;
		}
	}

	/* Restore the key */
	for (idx = 0; idx < k; idx++) {
		memcpy(&keyshares[idx], &shares[idx],
		       sss_KEYSHARE_LEN);
	}
	sss_combine_keyshares(key, (const sss_Keyshare*) keyshares, k);

	/* Decrypt the ciphertext */
	memcpy(&c[crypto_secretbox_BOXZEROBYTES],
	       &shares[0][sss_KEYSHARE_LEN], sss_CLEN);
	ret |= crypto_secretbox_open(m, c, clen, nonce, key);
	memcpy(data, &m[crypto_secretbox_ZEROBYTES], sss_MLEN);

	return ret;
}

/*
 * Combine `k` shares pointed to by `shares` and write the result to `data`
 *
 * This function returns -1 if any of the shares were corrupted or if the number
 * of shares was too low. It is not possible to detect which of these errors
 * did occur.
 */
int sss_combine_shares_varlen(uint8_t *data, const uint8_t *shares,
	                      const size_t share_len, uint8_t k)
{
	unsigned char key[crypto_secretbox_KEYBYTES];
	unsigned char c[crypto_secretbox_BOXZEROBYTES + sss_VARIABLE_MAX_CLEN] = { 0 };
	unsigned char m[crypto_secretbox_ZEROBYTES + sss_VARIABLE_MAX_CLEN];
	sss_Keyshare keyshares[k];
	size_t idx, offset;
	int ret = 0;

	if (share_len < sss_KEYSHARE_LEN + 16) {
		return -1;
	}
	if (share_len > sss_VARIABLE_MAX_SHARE_LEN) {
		return -1;
	}
	const size_t data_len = share_len - sss_KEYSHARE_LEN;

	/* Check if all ciphertexts are the same */
	if (k < 1) return -1;
	for (idx = 1; idx < k; idx++) {
		offset = idx * share_len;
		if (memcmp(get_varlen_ciphertext_const(&shares[0]),
			   get_varlen_ciphertext_const(&shares[offset]), data_len) != 0) {
			return -1;
		}
	}

	for (idx = 0; idx < k; idx++) {
		offset = idx * share_len;
		memcpy(&keyshares[idx], &shares[offset], sss_KEYSHARE_LEN);
	}
	sss_combine_keyshares(key, (const sss_Keyshare*) keyshares, k);

	/* Decrypt the ciphertext */
	unsigned long long clen = crypto_secretbox_BOXZEROBYTES + data_len;
	unsigned long long mlen = data_len - 16;
	memcpy(&c[crypto_secretbox_BOXZEROBYTES],
	       &(&shares[0])[sss_KEYSHARE_LEN], data_len);
	ret |= crypto_secretbox_open(m, c, clen, nonce, key);
	memcpy(data, &m[crypto_secretbox_ZEROBYTES], mlen);

	return ret;
}
