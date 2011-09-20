#ifndef __glth_fir_h
#define __glth_fir_h

/* glth_fir.h
 * written by jared kofron <jared.kofron@gmail.com>
 * implements a finite impulse response filter structure for goliath.
 */

// sys includes
#include <string.h>

// goliath includes
#include "glth_cb.h"

/* 
 * the main struct.  encapsulates everything a FIR needs to do its black
 * magic - a delay line in the form of a circular buffer, and a 
 * set of coefficients.
 */
typedef struct {
  unsigned int ntaps;
  glth_cb* delay;
  glth_sample_t* coefs;
} glth_fir;

/*
 * initializes a glth_fir for use.  initializes the delay line and the 
 * coefficients from an array in the arguments.
 */
glth_result glth_fir_init(glth_fir*, const unsigned int, glth_sample_t*);

/* 
 * update a glth_fir, placing the result in the third argument.
 */
glth_result glth_fir_update(glth_fir*, 
			    const glth_sample_t, 
			    glth_sample_t*);

/*
 * calculate the filter output over its current delay line and 
 * coefficients.
 */
glth_sample_t glth_fir_output(glth_fir* filter);

#endif __glth_fir_h
