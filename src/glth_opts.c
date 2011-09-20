#include "glth_opts.h"

glth_result args_to_opts(glth_opts* tgt, int argc, char** argv) {
  glth_result res = glth_success;
  
  if( tgt != NULL ) {
    int c;
    opterr = 0;

    while( (c = getopt(argc,argv,"hi:w:")) != -1 ) {
      switch(c) {
	//input file name
      case 'i':
	tgt->in_file_name = malloc(strlen(optarg));
	strcpy(tgt->in_file_name,optarg);
	set_bit(tgt,input_file_set);
	break;
      case 'h':
	printf("help!\n");
	break;
      case 'w':
	if( sscanf(optarg,"%u",&(tgt->filter_width)) == 0 ) {
	  printf("help!\n");
	  res = glth_badarg;
	  c = -1;
	}
	else set_bit(tgt,filter_width_set);
	break;
      }
    }
    set_defaults(tgt);
    if( check_opts(tgt) != glth_success) res = glth_failure;
  }
  else res = glth_nullptr;

  return res;
}

glth_result check_opts(glth_opts* tgt) {
  glth_result res = glth_success;

  if( tgt-> filter_width <= 0 ) {
    res = glth_badarg;
  }

  if( access(tgt->in_file_name,R_OK) != 0 ) {
    res = glth_badarg;
  }

  return res;
}


glth_result set_defaults(glth_opts* tgt) {
  glth_result res = glth_success;
  if(tgt != NULL) {
    if( !chk_bit(tgt,input_file_set) ) {
      tgt->in_file_name = "input.egg";
    }
    if( !chk_bit(tgt,filter_width_set) ) {
      tgt->filter_width = 1024;
    }
  }
  else res = glth_nullptr;

  return res;
}

glth_result print_glth_opts(glth_opts* tgt) {
  glth_result res = glth_success;
  if( tgt != NULL) {
    // check if input file name specified by user
    printf("\tinput file name: ");
    if( chk_bit(tgt,input_file_set) ) {
      printf("(u)");
    }
    printf("%s\n",tgt->in_file_name);

    // check if filter width set by user
    printf("\tfilter width: ");
    if( chk_bit(tgt,filter_width_set) ) {
      printf("(u)");
    }
    printf("%d\n",tgt->filter_width);
  }
  else res = glth_nullptr;

  return res;
}
