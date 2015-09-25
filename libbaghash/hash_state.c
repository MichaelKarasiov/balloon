

#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "constants.h"
#include "compress.h"
#include "errors.h"
#include "hash_state.h"

int 
hash_state_init (struct hash_state *s, struct baghash_options *opts,
    const void *salt, size_t saltlen)
{
  s->n_blocks = options_n_blocks (opts);
  s->block_size = options_block_size (opts);
  s->opts = opts;

  // TODO: Make sure this multiplication doesn't overflow (or use 
  // calloc or realloc)
  s->buffer = malloc (s->n_blocks * s->block_size);

  int error;
  if ((error = bitstream_init_with_seed (&s->bstream, salt, saltlen)))
    return error;

  return (s->buffer) ?  ERROR_NONE : ERROR_MALLOC;
}

void 
hash_state_free (struct hash_state *s)
{
  free (s->buffer);
}

int 
hash_state_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen)
{
  int error;
  struct bitstream bits;
  if ((error = bitstream_init (&bits)))
    return error;
  if ((error = bitstream_seed_add (&bits, in, inlen)))
    return error;
  if ((error = bitstream_seed_add (&bits, salt, saltlen)))
    return error;
  if ((error = bitstream_seed_finalize (&bits)))
    return error;

  const size_t buflen = s->n_blocks * s->block_size;
  if ((error = bitstream_fill_buffer (&bits, s->buffer, buflen)))
    return error;

  return ERROR_NONE;
}

static inline void *
block_index (const struct hash_state *s, size_t i)
{
  return s->buffer + (s->block_size * i);
}

static inline void *
block_last (const struct hash_state *s)
{
  return block_index (s, s->n_blocks - 1);
}

int 
hash_state_mix (struct hash_state *s)
{
  int error;
  unsigned char tmp_block[s->block_size];
  size_t neighbor;
  
  // Simplest design: hash in place with one buffer
  for (size_t i = 0; i < s->n_blocks; i++) {
    void *cur_block = block_index (s, i);

    const unsigned char *blocks[s->opts->n_neighbors + 1];

    // Hash in the previous block (or the last block if this is
    // the first block of the buffer).
    const unsigned char *prev_block = i ? cur_block - s->block_size : block_last (s);
    blocks[0] = prev_block;

    // For each block, pick random neighbors
    for (size_t n = 1; n < s->opts->n_neighbors; n++) { 

      // Get next neighbor
      if ((error = bitstream_rand_int (&s->bstream, &neighbor, s->n_blocks)))
        return error;
      blocks[n] = block_index (s, neighbor);
    }

    // Hash value of neighbors into temp buffer.
    if ((error = compress (tmp_block, blocks, s->opts->n_neighbors, s->block_size)))
      return error;

    // Copy output of compression function back into the 
    // big memory buffer.
    memcpy (cur_block, tmp_block, s->block_size);
    //printf("copy %p => %p\n", s->buffer + (s->block_size * i), tmp_block);
  }

  return ERROR_NONE;
}

int 
hash_state_extract (struct hash_state *s, void *out, size_t outlen)
{
  // For one-buffer design, just return the last block of the buffer
  memset (out, 0, outlen);
  memcpy (out, block_last (s), s->block_size);
  return ERROR_NONE;
}

