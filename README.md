# Shamir secret sharing library

[![Build Status](https://travis-ci.org/dsprenkels/sss.svg?branch=master)](https://travis-ci.org/dsprenkels/sss)

`sss` is a library that exposes an API to split secret data buffers into
a number of different _shares_. With the possession of some or all of these
shares, the original secret can be restored. It is the schoolbook example of
a cryptographic _threshold scheme_. This library has a [command line
interface][sss-cli]. ([web demo])

[sss-cli]: https://github.com/dsprenkels/sss-cli

## Table of contents

1. [Introduction](#introduction)
2. [Download](#download)
3. [Usage](#usage)
	1. [Example](#example)
4. [Bindings](#bindings)
5. [Technical details](#technical-details)
6. [Comparison of secret sharing libraries](#comparison-of-secret-sharing-libraries)
7. [Questions](#questions)

## Introduction

An example use case is a beer brewery which has a vault which contains their
precious super secret recipe. The 5 board members of this brewery do not trust
all the others well enough that they won't secretly break into the vault and
sell the recipe to a competitor. So they split the code into 5 shares, and
allow 4 shares to restore the original code. Now they are sure that the
majority of the staff will know when the vault is opened, but they can still
open the vault when one of the staff members is abroad or sick at home.

As often with crypto libraries, there is a lot of Shamir secret sharing code
around that *does not meet cryptographic standards* (a.k.a. is insecure).
Some details—like integrity checks and side-channel resistance—are often
forgotten. But these slip-ups can often fully compromise the security of the
scheme.
With this in mind, I have made this library to:
- Be side channel resistant (timing, branch, cache)
- Secure the shared secret with a MAC
- Use the platform (OS) randomness source

It should be safe to use this library in "the real world". I currently regard
the API as being stable. Should there be any breaking changes, then I will
update the version number conforming to the [semantic versioning spec][semver].

[semver]: http://semver.org/

## Download

I have released version 0.1.0 of this library, which can be downloaded from
the [releases](https://github.com/dsprenkels/sss/releases) page. However, I
actually recommend cloning the library with git, to also get the necesarry
submodules:

```shell
git clone --recursive https://github.com/dsprenkels/sss.git
```

The current version is version 0.1.0, which should be stable enough for now.
The functionality may still change before version 1.0.0, although I will
still fix any security issues before that.

## Usage

Secrets are provided as arrays of 64 bytes long. This should be big enough to
store generally small secrets. If you wish to split larger chunks of data, you
can use symmetric encryption and split the key instead. Shares are generated
from secret data using `sss_create_shares` and shares can be combined again
using the `sss_combine_shares` functions. The shares are octet strings of
113 bytes each.

This library is implemented in such a way that the maximum number of shares
is 255.

Moreover, every share includes an ID, which is implemented as a counter.
This ID is not considered a secret by the library, and an participants may be
able to infer the amount of shares from these ids (for example, if I have a
share with ID=3, I expect that ID∈{1,2} will also exist.
If you require random share IDs, then you should generate 255 different
shares, and randomly throw away the excess shares.

### Example

```c
#include "sss.h"
#include "randombytes.h"
#include <assert.h>
#include <string.h>

int main()
{
	uint8_t data[sss_MLEN], restored[sss_MLEN];
	sss_Share shares[5];
	size_t idx;
	int tmp;

	// Read a message to be shared
	strncpy(data, "Tyler Durden isn't real.", sizeof(data));

	// Split the secret into 5 shares (with a recombination theshold of 4)
	sss_create_shares(shares, data, 5, 4);

	// Combine some of the shares to restore the original secret
	tmp = sss_combine_shares(restored, shares, 4);
	assert(tmp == 0);
	assert(memcmp(restored, data, sss_MLEN) == 0);
}
```

## Bindings

I have currently written bindings for the following languages:

- [Node.js](https://github.com/dsprenkels/sss-node)
- [Go](https://github.com/dsprenkels/sss-go)
- [Rust](https://github.com/dsprenkels/sss-rs)
- [WASM](https://github.com/3box/sss-wasm)
- [Android](https://github.com/dsprenkels/sss-android)¹
- [Haskell](https://github.com/dsprenkels/sss-hs)¹
- [Swift](https://github.com/dsprenkels/sss-swift)¹
> ¹ No releases yet.

## Technical details

Shamir secret sharing works by generating a polynomial (e.g. _33x³ + 8x² + 29x +
42_). The lowest term is the secret and is just filled in. All the
other terms are generated randomly. Then we can pick points on the polynomial
by filling in values for _x_. Each point is put in a share. Afterwards, with _k_
points we can use interpolation to restore a _k_-degree polynomial.

In practice there is a wrapper around the secret-sharing part (this is done
because of crypto-technical reasons). This wrapper uses the XSalsa20/Poly1305
authenticated encryption scheme. Because of this, the shares are always a little
bit larger than the original data.

This library uses a custom [`randombytes`][randombytes] function to generate a
random encapsulation key, which talks directly to the operating system. When
using the high level API, you are not allowed to choose your own key. It _must_
be uniformly random, because regularities in shared secrets can be exploited.

With the low level API (`hazmat.h`) you _can_ choose to secret-share a piece of
data of exactly 32 bytes. This produces a set of shares that are much shorter
than the high-level shares (namely 33 bytes each). However, keep in mind that
this module is called `hazmat.h` (for "hazardous materials") for a reason.
Please only use this if you _really_ know what you are doing. Raw "textbook"
Shamir secret sharing is only safe when using a uniformly random secret (with
128 bits of entropy). Note also that it is entirely insecure for integrity.
Please do not use the low-level API unless you _really_ have no other choice.

## Comparison of secret-sharing libraries

If you would like your library to be added here, please open a pull request. :)

| Library         | Side-channels | Tamper-resistant | Secret length |
|-----------------|---------------|------------------|---------------|
| [B. Poettering] | Insecure¹     | Insecure         | 128 bytes     |
| [libgfshare]    | Insecure²     | Insecure         | ∞             |
| [blockstack]    | ??³           | Insecure         | 160 bytes     |
| [sssa-golang]   | Secure        | Secure⁴          | ∞             |
| [sssa-ruby]     | ??³           | Secure⁴          | ∞             |
| [snipsco]       | Secure        | Insecure         | Note⁶         |
| [c-sss]         | Insecure⁷     | Insecure         | ∞             |
| [timtiemens]    | Insecure⁸     | Note⁹            | 512 bytes     |
| [dsprenkels]    | Secure        | Secure⁵          | 64 bytes      |

### Notes

It is important to note that a limited secret length does not mean
that it is impossible to share longer secrets. The way this is done is
by secret sharing a random key and using this key to encrypt the real
secret. This is a lot faster and the security is not reduced. (This is
actually how [sss-cli] produces variable-length shares.)

1. Uses the GNU gmp library.
2. Uses lookup tables for GF(256) multiplication.
3. This library is implemented in a high level scripting library which does not
   guarantee that its basic operators execute in constant-time.
4. Uses randomized *x*-coordinates.
5. Uses randomized *y*-coordinates.
6. When using the [snipsco] library you will have to specify your own prime.
   Computation time is _O(p²)_, so on a normal computer you will be limited to
   a secret size of ~1024 bytes.
7. As mentioned by the [documentation](https://github.com/fletcher/c-sss#security-issues).
8. Uses Java `BigInteger` class.
9. Basic usage of this tool does not protect the integrity of the secrets.
   However, the project's readme file advises the user to use a hybrid
   encryption scheme and secret share the key. Through destroying the ephemeral
   key, the example that is listed in the readme file protects prevents an
   attacker from arbitrarily inserting a secret. However, inserting a garbled
   secret is still possible. To prevent this the user should use a AEAD scheme
   (like AES-GCM or ChaCha20-Poly1305) instead of AES-CBC.

[B. Poettering]: http://point-at-infinity.org/ssss/
[libgfshare]: https://github.com/jcushman/libgfshare
[blockstack]: https://github.com/blockstack/secret-sharing
[sssa-golang]: https://github.com/SSSaaS/sssa-golang
[sssa-ruby]: https://github.com/SSSaaS/sssa-ruby
[snipsco]: https://github.com/snipsco/rust-threshold-secret-sharing
[c-sss]: https://github.com/fletcher/c-sss
[timtiemens]: https://github.com/timtiemens/secretshare
[dsprenkels]: https://github.com/dsprenkels/sss


## Questions

### I do not know a lot about secret sharing. Is Shamir secret sharing useful for me?

It depends. In the case of threshold schemes (that's what this is) there are
two types:

1. The share-holders _cannot_ verify that their shares are valid.
2. The share-holders _can_ verify that their shares are valid.

Shamir's scheme is of the first type. This immediately implies that the dealer
could cheat. Indeed, they can distribute a number of shares which are just
random strings. The only way the participants could know is by banding together
and trying to restore the secret. This would show the secret, which would make
the scheme totally pointless.

**Use Shamir secret sharing only if the dealer _and_ the participants have no
reason to corrupt any shares.**

Examples where this is _not_ the case:

- When the secret hides something that is embarrasing for one of the
  participants.
- When the shared secret is something like a testament, and the participants
  are the heirs. If one of the heirs inherits more wealth when the secret is
  not disclosed, they can corrupt their share (and it would be impossible to
  check this from the share alone).

In these cases, you will need a scheme of the second type. See the next
question.

### Wait, I need verifiable shares! What should I use instead?

There are two straightforward options:

1. When the secret is fully random—for example, a cryptographic key—use
   **Feldman verifiable secret sharing**.
2. When the secret is not fully random—it _could_ be a message, a number,
   etc.—use **Pedersen verifiable secret sharing**.

### Other

For other questions, feel free to open an issue or send me an email on my Github
associated e-mail address.

[web demo]: http://bakaoh.com/sss-wasm/
[randombytes]: https://github.com/dsprenkels/randombytes
