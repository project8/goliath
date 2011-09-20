#include "glth_cb.h"

glth_result glth_cb_init(glth_cb* tgt, const unsigned int cb_size) {
  glth_result res = glth_success;
  if(tgt) {
    tgt->buf = (glth_sample_t*)calloc(cb_size,sizeof(glth_sample_t));
    if(tgt->buf == NULL) res = glth_e_nomem;
  }

  return res;
}

glth_result glth_cb_push(glth_cb* tgt, const glth_sample_t val) {
  glth_result res = glth_success;
  if(tgt != NULL) {
    tgt->buf[0] = val;
    for(int ii = (tgt->size) - 2; ii >= 0; ii--) {
      tgt->buf[ii+1] = tgt->buf[ii];
    }
  }
  else res = glth_nullptr;
  return res;
}
