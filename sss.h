/*
 * Intermediate level API for Daan Sprenkels' Shamir secret sharing library
 * Copyright (c) 2017 Daan Sprenkels <hello@dsprenkels.com>
 */


#ifndef SSS_SSS_H_
#define SSS_SSS_H_

#include "hazmat.h"
#include "tweetnacl.h"
#include <inttypes.h>


#ifndef SSS_MLEN
/*
Length of the message (must be known at compile-time)
*/
#define SSS_MLEN sizeof(uint8_t[64])
#endif


/*
 * Length of the ciphertext, including the message authentication code
 */
#define SSS_CLEN (crypto_secretbox_ZEROBYTES + SSS_MLEN - crypto_secretbox_BOXZEROBYTES)


/*
 * One share of a secret which is shared using Shamir's
 * the `SSS_create_shares` function.
 */
typedef struct {
	SSS_Keyshare keyshare;
	unsigned char c[SSS_MLEN];
} SSS_Share;


/*
 * Create `n` shares of the secret data `data`. Share such that `k` or more
 * shares will be able to restore the secret.
 *
 * This function will put the resulting shares in the array pointed to by
 * `out`. The caller has to guarantee that this array will fit at least `n`
 * instances of `SSS_Share`.
 */
void SSS_create_shares(SSS_Share *out,
                       const uint8_t *data,
                       uint8_t n,
                       uint8_t k);


/*
 * Combine the `k` shares pointed to by `shares` and put the resulting secret
 * data in `data`. The caller has to ensure that the `data` array will fit
 * at least `SSS_MLEN` (default: 64) bytes.
 *
 * On success, this function will return 0. If combining the secret fails,
 * this function will return a nonzero return code. On failure, the value
 * in `data` may have been altered, but must still be considered secret.
 */
int SSS_combine_shares(uint8_t *data,
                       const SSS_Share *shares,
                       uint8_t k);


#endif /* SSS_SSS_H_ */
