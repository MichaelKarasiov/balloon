/*
 * Copyright (c) 2015, Henry Corrigan-Gibbs
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <stdbool.h>
#include <stdint.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

// How many bytes to generate with AES at each invocation.
#define BITSTREAM_BUF_SIZE ((32) * (AES_BLOCK_SIZE))

struct bitstream {
  // All of the below are directly passed to OpenSSL
  bool initialized;
  SHA256_CTX c;
  EVP_CIPHER_CTX ctx;

  uint8_t *zeros;
  uint8_t *generated;

  uint8_t *genp;
  unsigned int n_refreshes;
};


int bitstream_init (struct bitstream *b);

int bitstream_free (struct bitstream *b);

int bitstream_init_with_seed (struct bitstream *b, const void *seed, size_t seedlen);

int bitstream_seed_add (struct bitstream *b, const void *seed, size_t seedlen);

int bitstream_seed_finalize (struct bitstream *b);

/** 
 * This function will zero out the "out" buffer before filling
 * it with pseudo-random bytes.
 */
int bitstream_fill_buffer (struct bitstream *b, void *out, size_t outlen);

/** 
 * Return a random integer in the "out" parameter in the range
 * [0, max) -- exclusive of max.
 */
int bitstream_rand_int (struct bitstream *b, uint64_t *out, uint64_t max);

/**
 * Sample outlen random integers [0, max) with replacement and
 * only return the distinct values. Stores the number of distinct 
 * elements returned in n_found.
 */
int bitstream_rand_ints_nodup (struct bitstream *b, uint64_t *outs, size_t *n_found, 
  size_t outlen, uint64_t max);

/**
 * Get a single char.
 */
int bitstream_rand_byte (struct bitstream *b, uint8_t *out);

#endif
