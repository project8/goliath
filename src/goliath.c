#include <stdio.h>
#include "glth_opts.h"

int main(const int argc, char** argv) {
  // first initialize the run information struct
  glth_opts* run_info = (glth_opts*)calloc(1,sizeof(glth_opts));
  if( args_to_opts(run_info, argc, argv) != glth_success ) {
    printf("help!\n");
  }
  else print_glth_opts(run_info);

  // done.
  free(run_info);
  return glth_success;
}
