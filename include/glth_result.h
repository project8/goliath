#ifndef __glth_result_h
#define __glth_result_h

/* glth_result.h
 * written by jared kofron <jared.kofron@gmail.com>
 * result codes for the goliath program.
 */
typedef short glth_result;

/*
 * values to be returned upon function success or
 * failure.
 */
static const glth_result glth_success = 0x0;
static const glth_result glth_failure = 0x1;

/*
 * this is returned if memory allocation fails for
 * something.
 */
static const glth_result glth_e_nomem = 0x2;

/* 
 * return if a null pointer was passed to a function
 * inappropriately.
 */
static const glth_result glth_nullptr = 0x3;

/*
 * return if an argument to a function is inappropriate.
 */
static const glth_result glth_badarg = 0x4;

#endif //__glth_result_h
