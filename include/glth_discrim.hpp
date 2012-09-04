#ifndef __glth_discrim_hpp
#define __glth_discrim_hpp

/*
 * The glth_discrim class abstracts away the concept of a
 * "discriminator", which is to say a bit or array of bits
 * that signify the presence or absence of some condition.
 * discriminator supports checking a single bit by using the
 * [] operator, or it can check a range of bits (OR'ed) by
 * using the any(low,high) function.
 */

#include <deque>

namespace glth {

  class discriminator {
  private:
    std::deque<bool> _bank;
  public:
    discriminator(std::size_t);
    bool& operator[](std::size_t index);
    bool any(std::size_t low, std::size_t high);
  }; // class discriminator

}; // namespace glth

#endif // __glth_discrim_hpp
