#define _GNU_SOURCE

#include "hazmat.h"
#include <assert.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>


/* Use Rijndael polynomial to reduce values in GF(2^8) */
#define REDUCE_POLY 0x1b

typedef struct {
	uint8_t x;
	uint8_t y;
} ByteShare;


/*
 * Safely multiply two polynomials in GF(2^8)
 */
static uint8_t _gf256_mul(uint8_t a, uint8_t b)
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
static uint8_t _gf256_inv(uint8_t a)
{
	size_t idx;
	uint8_t ret = a;
	/* Use square-multiply to calculate a^254 */
	for (idx = 0; idx < 6; idx++) {
		ret = _gf256_mul(ret, ret);
		ret = _gf256_mul(ret, a);
	}
	ret = _gf256_mul(ret, ret);
	return ret;
}


/*
 * Create the ByteShares for one secret byte
 */
static int create_byte_share(ByteShare *out,
                           uint8_t secret,
                           uint8_t n, uint8_t k)
{
	uint8_t poly[256] = { 0 }, x, y, xpow;
	size_t point_idx, coeff_idx;
	int tmp;

	/* Check if the parameters are valid */
	if (n == 0) return -1;
	if (k == 0) return -1;
	if (k > n) return -1;

	/* Create a random polynomial of order k */
	tmp = syscall(SYS_getrandom, &poly[255 - k], k, 0);
	assert(tmp == k); /* Failure indicates a bug in the code */

	/* Set the secret value in the polynomial */
	poly[255] = secret;

	for (coeff_idx = 0; coeff_idx < 256; coeff_idx++) {
		printf("%02hhx", poly[coeff_idx]);
	}
	printf("\n");

	/* Generate some points for x = 1..n that are on the polynomial */
	for (point_idx = 0; point_idx < n; point_idx++) {
		x = point_idx + 1;
		y = 0;
		xpow = 1;
		for (coeff_idx = 0; coeff_idx < k; coeff_idx++) {
			y ^= _gf256_mul(xpow, poly[255 - coeff_idx]);
			xpow = _gf256_mul(xpow, x);
		}
		out[point_idx].x = x;
		out[point_idx].y = y;
	}
	return 0;
}


/*
 * Restore a secret byte from `k` bytes shares in `shares`
 */
static uint8_t combine_byte_share(const ByteShare *shares, const uint8_t k)
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
			num = _gf256_mul(num, shares[idx2].x);
			denom = _gf256_mul(denom, shares[idx1].x ^ shares[idx2].x);
		}
		basis_poly = _gf256_mul(num, _gf256_inv(denom));
		/* Add scaled polynomial coefficient to restored secret */
		secret ^= _gf256_mul(shares[idx1].y, basis_poly);
	}
	return secret;
}
