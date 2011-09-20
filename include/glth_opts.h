#ifndef __glth_opts_h
#define __glth_opts_h

// sys includes
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

// glth includes
#include "glth_result.h"

// bitmask defines
#define chk_bit(info,bit) ((info->conf_mask & bit) == bit)
#define set_bit(info,bit) ((info->conf_mask |= bit))

/*
 * the width of the desired north filter.
 */
const static short filter_width_set = 0x1;

/*
 * the filename that goliath should look in for data.
 */
const static short input_file_set = 0x2;

/*
 * the options struct.  keeps things nice and tidy.
 */
typedef struct {
  unsigned int filter_width;
  unsigned char conf_mask;
  char* in_file_name;
} glth_opts;

/*
 * parse the input arguments into a glth_opts struct.
 * returns success if everything worked out.
 */
glth_result args_to_opts(glth_opts*, int argc, char** argv);

/*
 * sets defaults for things.
 */
glth_result set_defaults(glth_opts*);

/*
 * checks valid settings for options.
 */
glth_result check_opts(glth_opts* tgt);

/*
 * pretty prints options to screen.
 */
glth_result print_glth_opts(glth_opts*);

#endif // __glth_opts_h
