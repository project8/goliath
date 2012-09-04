#ifndef __glth_env_hpp
#define __glth_env_hpp

#include <string>
#include <stdint.h>

#include "glth_const.hpp"

namespace glth {

  // configuration bitmasks
  typedef uint64_t bitmask;
  static bitmask _name_set = (2 << 0);

  class env {
  private:
    // Default constructor is private because why do you need that?
    env();
    std::string _in_filename;
    uint64_t _conf_mask;

  public:
    // Factory method from arguments
    static env const* env_from_args(int argc, char** argv);

    // Verify that an environment is sensible e.g. that required fields
    // have been set.
    static glth_const::env_result verify(const glth::env);

    // getters/setters
    void set_in_filename(std::string);
    std::string get_in_filename() const;
    std::string get_in_filename();
  };

}; // namespace glth

#endif // __glth_env_hpp
