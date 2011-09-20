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
