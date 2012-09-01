#ifndef __glth_io_hpp
#define __glth_io_hpp

#include <string>
#include "Monarch.hpp"
#include "glth_const.hpp"

namespace glth {

  /*
   * goliath's IO class is a wrapper around a monarch pointer whose
   * job it is to retrieve data from a monarch instance and deliver
   * it into a signal.
   */
  class io {
  private:
    io();
    Monarch const* _monarch_ptr;
  public:
    static io* open_file(std::string filename);
  }; // class io

}; // namespace glth

#endif
