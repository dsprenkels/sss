#include "hazmat.h"
#include "tweetnacl.h"
#include <assert.h>
#include <string.h>


/* Use Rijndael polynomial to reduce values in GF(2^8) */
#define REDUCE_POLY 0x1b

typedef struct {
	uint8_t x;
	uint8_t y;
} ByteShare;


extern void FIPS202_SHAKE256(const unsigned char *in, unsigned long long inLen,
	                     unsigned char *out, unsigned long long outLen);


/*
 * Safely multiply two polynomials in GF(2^8)
 */
static uint8_t gf256_mul(uint8_t a, uint8_t b)
{
	size_t idx;
	uint8_t do_reduce, ret = 0;

	for (idx = 0; idx < 8; idx++) {
		ret ^= (b & 1) * a;
		do_reduce = a >> 7;
		a <<= 1;
		a ^= do_reduce * REDUCE_POLY;
		b >>= 1;
	}
	return ret;
}


/*
 * Invert `a` in GF(2^8)
 */
static uint8_t gf256_inv(uint8_t a)
{
	size_t idx;
	uint8_t ret = a;
	/* Use square-multiply to calculate a^254 */
	for (idx = 0; idx < 6; idx++) {
		ret = gf256_mul(ret, ret);
		ret = gf256_mul(ret, a);
	}
	ret = gf256_mul(ret, ret);
	return ret;
}


/*
 * Create the ByteShares for one secret byte
 */
static int create_byte_shares(ByteShare *out,
                             uint8_t secret,
                             uint8_t n, uint8_t k,
                             uint8_t *random_bytes)
{
	uint8_t poly[256] = { 0 }, x, y, xpow;
	size_t point_idx, coeff_idx;

	/* Check if the parameters are valid */
	if (n == 0) return -1;
	if (k == 0) return -1;
	if (k > n) return -1;

	/* Create a random polynomial of order k */
	memcpy(&poly[255 - k], random_bytes, k);

	/* Set the secret value in the polynomial */
	poly[255] = secret;

	/* Generate some points for x = 1..n that are on the polynomial */
	for (point_idx = 0; point_idx < n; point_idx++) {
		x = point_idx + 1;
		y = 0;
		xpow = 1;
		for (coeff_idx = 0; coeff_idx < k; coeff_idx++) {
			y ^= gf256_mul(xpow, poly[255 - coeff_idx]);
			xpow = gf256_mul(xpow, x);
		}
		out[point_idx].x = x;
		out[point_idx].y = y;
	}
	return 0;
}


/*
 * Restore a secret byte from `k` bytes shares in `shares`
 */
static uint8_t combine_byte_shares(const ByteShare *shares, const uint8_t k)
{
	/* Restore the least significant coefficient in the original poly */
	size_t idx1, idx2;
	uint8_t secret = 0, num, denom, basis_poly;

	/* Use Lagrange basis polynomials to calculate the secret coefficient */
	for (idx1 = 0; idx1 < k; idx1++) {
		num = 1;
		denom = 1;
		for (idx2 = 0; idx2 < k; idx2++) {
			if (idx1 == idx2) continue;
			num = gf256_mul(num, shares[idx2].x);
			denom = gf256_mul(denom, shares[idx1].x ^ shares[idx2].x);
		}
		basis_poly = gf256_mul(num, gf256_inv(denom));
		/* Add scaled polynomial coefficient to restored secret */
		secret ^= gf256_mul(shares[idx1].y, basis_poly);
	}
	return secret;
}


/*
 * Create `k` key shares of the key given in `key`. The caller has to ensure that
 * the array `out` has enough space to hold at least `n` sss_Keyshare structs.
 */
 void sss_create_keyshares(sss_Keyshare *out,
                           const uint8_t key[32],
                           uint8_t n,
                           uint8_t k)
{
	size_t byte_idx, share_idx;
	uint8_t x;
	ByteShare byte_shares[n * sizeof(ByteShare)];
	uint8_t random_bytes[k * 256];

	/* Generate a lot of random bytes */
	FIPS202_SHAKE256(key, 32, random_bytes, sizeof(random_bytes));

	for (share_idx = 0; share_idx < n; share_idx++) {
		x = share_idx + 1;
		out[share_idx].x = x;
	}

	for (byte_idx = 0; byte_idx < 32; byte_idx++) {
		create_byte_shares(byte_shares, key[byte_idx], n, k,
		                   &random_bytes[byte_idx * k]);
		for (share_idx = 0; share_idx < n; share_idx++) {
			assert(out[share_idx].x == byte_shares[share_idx].x);
			out[share_idx].y[byte_idx] = byte_shares[share_idx].y;
		}
	}
}

/*
 * Restore the `k` sss_Keyshare structs given in `shares` and write the result
 * to `key`.
 */
 void sss_combine_keyshares(uint8_t key[32],
                            const sss_Keyshare *key_shares,
                            uint8_t k)
{
	size_t byte_idx, share_idx;
	ByteShare byte_shares[k * sizeof(ByteShare)];

	for (share_idx = 0; share_idx < k; share_idx++) {
		byte_shares[share_idx].x = key_shares[share_idx].x;
	}

	for (byte_idx = 0; byte_idx < 32; byte_idx++) {
		for (share_idx = 0; share_idx < k; share_idx++) {
			byte_shares[share_idx].y = key_shares[share_idx].y[byte_idx];
		}
		key[byte_idx] = combine_byte_shares(byte_shares, k);
	}
}
