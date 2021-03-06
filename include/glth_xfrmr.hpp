#ifndef __glth_xfrmr_hpp
#define __glth_xfrmr_hpp

#include "glth_signal.hpp"
extern "C" {
  #include "fftw3.h"
}

namespace glth {

  class glth_xfrmr {
  private:
    std::size_t _in_size;
    std::size_t _out_size;
    std::size_t _freq_res;
    std::complex<double>* _in;
    std::complex<double>* _out;
    fftw_plan _fwd_plan;

    int irem(double a, double b);

  public:
    glth_xfrmr(std::size_t in_size,
	       std::size_t out_size,
	       std::size_t resolution);

    // Calculate the wigner-ville distribution for an input signal.  
    // Equivalent to xwvd(tgt, tgt, out).
    void wvd(glth::analog_signal tgt, std::complex<double>** out);

    // Calculate the cross wigner-ville distribution for two signals.
    void xwvd(glth::analog_signal tgt1,
	      glth::analog_signal tgt2,
	      std::complex<double>** out);
  };

}; // namespace glth

#endif // __glth_xfrmr_hpp
