#include <iostream>
#include "glth_io.hpp"
#include "glth_env.hpp"
#include "glth_signal.hpp"

using glth::env;
using glth::io;
using namespace glth_const;

int main(int argc, char** argv) {
  // By default we have succeeded.
  exit_status res = exit_success;

  // OK now parse the arguments passed by the user and
  // verify that they are sensible.
  env const* env = env::env_from_args(argc, argv);

  if( env::verify(*env) != env_success ) {
    std::cout << "failed to create valid environment!" << std::endl;
    res = exit_env_failed;
  }

  else {
    // Our pointer to the IO object which gets our data for us
    io* io = io::open_file(env->get_in_filename());

    // Signals!
    std::size_t record_len = (io->get_record_length());
    std::cout << "*Record length: " << record_len << std::endl;
    std::cout << "**Allocating channel signals" << std::endl;
    glth::signal ch1(record_len), ch2(record_len);

    if(io) {
      std::cout << "hi" << std::endl;
    }

    // Otherwise opening the file barfed.  Exit ungracefully.
    else {
      std::cout << "couldn't open file: " 
		<< env->get_in_filename() 
		<< std::endl;
      res = exit_io_failed;
    }
  }

  return res;
}
