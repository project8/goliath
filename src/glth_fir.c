#include "glth_fir.h"

glth_result glth_fir_init(glth_fir* tgt,
			  const unsigned int ntaps,
			  glth_sample_t* coefset) {
  glth_result res = glth_success;
  if(tgt != NULL) {
    if( glth_cb_init(tgt->delay,ntaps) == glth_success ) {
      tgt->coefs = (glth_sample_t*)calloc(ntaps, sizeof(glth_sample_t));
      if(tgt->coefs != NULL) {
	memcpy(tgt->coefs,coefset,ntaps*sizeof(glth_sample_t));
      }
      else res = glth_e_nomem;
    }
    else res = glth_e_nomem;
  }
  else res = glth_nullptr;
  return res;
}

glth_result glth_fir_update(glth_fir* filter, 
			      const glth_sample_t val,
			      glth_sample_t* tgt) {
  glth_result res = glth_success;
  if(tgt && filter) {
    if(glth_cb_push(filter->delay,val) == glth_success) {
      (*tgt) = glth_fir_output(filter);
    }
    else res = glth_nullptr;
  }
  else res = glth_nullptr;
  return res;
}

glth_sample_t glth_fir_output(glth_fir* filter) {
  glth_sample_t accum = 0;
  if(filter) {
    for(unsigned int i = 0; i < filter->ntaps; i++) {
      accum += filter->coefs[i]*filter->delay->buf[i];
    }
  }
  return accum;
}
