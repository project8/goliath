#ifndef __glth_sig_hpp
#define __glth_sig_hpp

#include <vector>
#include <complex>
#include "glth_const.hpp"
#include "glth_types.hpp"
#include "glth_px1500.hpp"

namespace glth {

  template <glth_types::glth_signal_type T>
  class signal {
  private:
    std::vector<double> _sigdata;
    std::size_t _siglen;
    signal();
  public:
    glth_types::byte& operator[](std::size_t pos);
    std::size_t len();
  }; // default template class, private constructor

  // Some typedefs
  typedef glth::signal<glth_types::analog> analog_signal;
  typedef glth::signal<glth_types::digital> digital_signal;

  template <>
  class signal<glth_types::analog> {
  private:
    std::vector<std::complex<double> > _sigdata;
    std::size_t _siglen;
  public:
    // Constructor takes a single argument, the length of the signal
    signal(std::size_t len);

    // Array access operator
    std::complex<double>& operator[](std::size_t pos);

    std::size_t len();
    std::complex<double>* data();

  }; // class analog signal

  template <>
  class signal<glth_types::digital> {
  private:
    std::vector<glth_types::byte> _sigdata;
    std::size_t _siglen;
  public:
    // Constructor takes a single argument, the length of the signal
    signal(std::size_t len);

    // Array access operator
    glth_types::byte& operator[](std::size_t pos);
    glth_types::byte* data();

    std::size_t len();

  }; // class digital signal

  // Convert analog to digital and back.  Note that this
  // assumes 8 bit resolution b/c that's what makes sense given
  // the digital type.
  
  digital_signal* analog_to_digital(analog_signal in);

  analog_signal* digital_to_analog(digital_signal in);

} // namespace glth_signal

#endif // __glth_sig_hpp
