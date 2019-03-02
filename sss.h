/*
 * Intermediate level API for Daan Sprenkels' Shamir secret sharing library
 * Copyright (c) 2017 Daan Sprenkels <hello@dsprenkels.com>
 */


#ifndef sss_SSS_H_
#define sss_SSS_H_

#include "hazmat.h"
#include "tweetnacl.h"
#include <inttypes.h>
#include <unistd.h>


/*
Length of the message (must be known at compile-time)
*/
#define sss_MLEN sizeof(uint8_t[64])

/*
Length of the message (must be known at compile-time)
*/
#define sss_VARIABLE_MAX_MLEN sizeof(uint8_t[4096])

/*
 * Length of the ciphertext, including the message authentication code
 */
#define sss_CLEN (sss_MLEN + 16)

/*
 * Maximum length of the ciphertext, including the message authentication code
 */
#define sss_VARIABLE_MAX_CLEN (sss_VARIABLE_MAX_MLEN + 16)

/*
 * Length of a SSS share
 */
#define sss_SHARE_LEN (sss_CLEN + sss_KEYSHARE_LEN)

/*
 * Maximum length of a variable length share
 */
#define sss_VARIABLE_MAX_SHARE_LEN (sss_VARIABLE_MAX_CLEN + sss_KEYSHARE_LEN)

/*
 * One share of a secret which is shared using Shamir's
 * the `sss_create_shares` function.
 */
typedef uint8_t sss_Share[sss_SHARE_LEN];


/*
 * Create `n` shares of the secret data `data`. Share such that `k` or more
 * shares will be able to restore the secret.
 *
 * This function will put the resulting shares in the array pointed to by
 * `out`. The caller has to guarantee that this array will fit at least `n`
 * instances of `sss_Share`.
 */
void sss_create_shares(sss_Share *out,
                       const uint8_t *data,
                       uint8_t n,
                       uint8_t k);

/*
 * Create `n` shares of the secret data `data`. Share such that `k` or more
 * shares will be able to restore the secret.
 *
 * This function will put the resulting shares in the array pointed to by
 * `out`. The caller has to guarantee that this array will fit at least `n`
 * instances of `sss_Share`.
 *
 * This function allows for secret sharing variable-length messages.
 * However: **Messages are limited to 4096 bytes!**
 *
 * The caller MUST ensure that `out` points to an array that is at least
 * `sss_KEYSHARE_LEN + data_len + 16` bytes long! As of now, `sss_KEYSHARE_LEN`
 * is *33* bytes, so that means the `out` array must be `data_len + 49` bytes
 * long.
 *
 * This function will return 0 on success, otherwise it will return a nonzero
 * value. In this case your data was too long.
 */
int sss_create_shares_varlen(uint8_t *out,
                             const uint8_t *data,
		             const size_t data_len,
                             uint8_t n,
                             uint8_t k);


/*
 * Combine the `k` shares pointed to by `shares` and put the resulting secret
 * data in `data`. The caller has to ensure that the `data` array will fit
 * at least `sss_MLEN` (default: 64) bytes.
 *
 * On success, this function will return 0. If combining the secret fails,
 * this function will return a nonzero return code. On failure, the value
 * in `data` may have been altered, but must still be considered secret.
 */
int sss_combine_shares(uint8_t *data,
                       const sss_Share *shares,
                       uint8_t k);


/*
 * Combine the `k` shares pointed to by `shares` and put the resulting secret
 * data in `data`. The caller has to ensure that the `data` array will fit
 * at least `sss_VARIABLE_MAX_MLEN` (4096) bytes!
 *
 * On success, this function will return 0. If combining the secret fails,
 * this function will return a nonzero return code. On failure, the value
 * in `data` may have been altered, but must still be considered secret.
 */
int sss_combine_shares(uint8_t *data, const sss_Share *shares, uint8_t k);


/*
 * Combine `k` shares pointed to by `shares` and write the result to `data`
 *
 * This function allows for restoring variable length secrets. The caller MUST
 * ensure that `out` is at least `4096` bytes long.
 *
 * This function returns 0 on success. A nonzero value indicates an error,
 * this can be caused by multiple factors:
 *   - One or more of the shares were invalid or corrupted
 *   - There were not enough shares to recombine the secret
 *   - `share_len` was invalid (it must be in [49, 4145])
 */
int sss_combine_shares_varlen(uint8_t *data, const uint8_t *shares,
	                      const size_t share_len, uint8_t k);


#endif /* sss_SSS_H_ */
