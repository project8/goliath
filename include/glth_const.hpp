#ifndef __glth_const_hpp
#define __glth_const_hpp

namespace glth_const {
  typedef enum {
    exit_success = 0,
    exit_env_failed = 1,
    exit_io_failed = 2
  } exit_status;

  typedef enum {
    io_success = 0,
    io_read_ok = 1,
    io_read_badarray = 2,
    io_eof = 3,
    io_channel_err = 4
  } io_result;

  typedef enum {
    env_success = 0,
    env_no_filename = 1
  } env_result;

} // glth_const namespace

#endif // __glth_const_hpp
