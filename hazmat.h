/*
 * Low level API for Daan Sprenkels' Shamir secret sharing library
 * Copyright (c) 2017 Daan Sprenkels <hello@dsprenkels.com>
 *
 * Usage of this API is hazardous and is only reserved for beings with a
 * good understanding of the Shamir secret sharing scheme and who know how
 * crypto code is implemented. If you are unsure about this, use the
 * intermediate level API. You have been warned!
 */


#ifndef SSS_HAZMAT_H_
#define SSS_HAZMAT_H_

#include <inttypes.h>

typedef struct {
	const uint8_t x;
	const uint8_t y[32];
} SSS_Keyshare;


void SSS_create_keyshares(SSS_Keyshare *out,
                         const uint8_t key[32],
                         uint8_t n,
                         uint8_t k);


void SSS_combine_keyshares(uint8_t key[32],
                          const SSS_Keyshare *shares,
			  uint8_t k);


#endif /* SSS_HAZMAT_H_ */
