#ifndef __glth_discrim_hpp
#define __glth_discrim_hpp

/*
 * The glth_discrim class abstracts away the concept of a
 * "discriminator", which is to say a bit or array of bits
 * that signify the presence or absence of some condition.
 */

#include <vector>

namespace glth {

  class discriminator {
  private:
    std::vector<bool> _bank;
  public:
    discriminator(std::size_t);
    bool operator[](std::size_t index);
  }; // class discriminator

}; // namespace glth

#endif // __glth_discrim_hpp
