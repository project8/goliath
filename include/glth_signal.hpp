#ifndef __glth_sig_hpp
#define __glth_sig_hpp

#include <vector>
#include <complex>
#include "glth_const.hpp"
#include "glth_types.hpp"
#include "glth_px1500.hpp"

namespace glth {

  /*
   * goliath considers a 'signal' to be a complex valued vector.
   */
  typedef std::vector<std::complex<double> > signal;

} // namespace glth_signal

#endif // __glth_sig_hpp
