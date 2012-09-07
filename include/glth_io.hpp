#ifndef __glth_io_hpp
#define __glth_io_hpp

#include <string>
#include "Monarch.hpp"
#include "glth_const.hpp"
#include "glth_signal.hpp"
#include "glth_px1500.hpp"

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

    std::size_t get_record_length() const;
    double get_record_frequency() const;

    glth_const::io_result populate(signal& ch1, signal& ch2);

  }; // class io

}; // namespace glth

#endif
