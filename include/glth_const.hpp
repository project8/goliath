#ifndef __glth_const_hpp
#define __glth_const_hpp

namespace glth_const {
  typedef enum {
    exit_success = 0,
    exit_env_failed = 1,
    exit_io_failed = 2
  } exit_status;

  enum io_result {
    io_success = 0
  };

  typedef enum {
    env_success = 0,
    env_no_filename = 1
  } env_result;

} // glth_const namespace

#endif // __glth_const_hpp
