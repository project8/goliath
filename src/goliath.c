#include <stdio.h>
#include "glth_usage.h"
#include "glth_opts.h"
#include "glth_fir.h"

int main(const int argc, char** argv) {
  glth_result res = glth_success;

  // first initialize the run information struct
  glth_opts* run_info = (glth_opts*)calloc(1,sizeof(glth_opts));
  if( (res = args_to_opts(run_info, argc, argv)) != glth_success ) {
    glth_usage();
  }
  else print_glth_opts(run_info);

  // done.
  free(run_info);
  return res;
}
