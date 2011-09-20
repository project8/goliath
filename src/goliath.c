#include <stdio.h>
#include "glth_opts.h"
#include "glth_fir.h"

int main(const int argc, char** argv) {
  // first initialize the run information struct
  glth_opts* run_info = (glth_opts*)calloc(1,sizeof(glth_opts));
  if( args_to_opts(run_info, argc, argv) != glth_success ) {
    printf("help!\n");
  }
  else print_glth_opts(run_info);

  // just a test!
  glth_fir* filter = (glth_fir*)calloc(1,sizeof(glth_fir));
  glth_sample_t coefs[5] = {1.0,6.0,3.0,4.0,5.0};
  glth_sample_t sig[11] = {0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0};
  glth_sample_t output = (glth_sample_t)0;
  glth_fir_init(filter,5,coefs);
  for(unsigned int i = 0; i < 11; i++) {
    glth_fir_update(filter,sig[i],&output);
    printf("%f\n",output);
  }

  // done.
  free(run_info);
  return glth_success;
}
