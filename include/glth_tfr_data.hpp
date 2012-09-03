#ifndef __glth_tfr_data_hpp
#define __glth_tfr_data_hpp

#include <vector>
#include <complex>

namespace glth {

  class tfr_data {
  private:
    std::size_t _ntime, _nfreq;
    std::complex<double>** _data;
  public:
    tfr_data(std::size_t nt, std::size_t nf);

    // size access
    std::size_t get_ntime();
    std::size_t get_nfreq();


    // column access
    std::complex<double>* operator[](std::size_t colidx);
  };
  
};

#endif // __glth_tfr_data_hpp
