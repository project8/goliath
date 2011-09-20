/* glth_cb.c
 * written by jared kofron <jared.kofron@gmail.com>
 * circular buffer implementation for goliath.  used for the delay lines
 * that the glth_fir finite impulse response filters use.
 */

// sys includes
#include <stdlib.h>

// goliath includes.
#include "glth_sample.h"
#include "glth_result.h"

typedef struct {
  unsigned int size;
  unsigned int state;
  glth_sample_t* buf;
} glth_cb;

/*
 * initializes a circular buffer for use.  returns all is well if memory
 * allocation was OK.
 */
glth_result glth_cb_init(glth_cb*, const unsigned int);

/*
 * pushes a value onto a circular buffer.  error generated if a null pointer
 * is passed as the first argument.
 */
glth_result glth_cb_push(glth_cb*, const glth_sample_t);
