#include <iostream>
#include <fstream>
#include <cmath>
#include "glth_signal.hpp"

extern "C" {
  #include "fftw3.h"
}

int irem( double x, double y)
{
 int result;

 if (y != 0)
   {
     result =  x-y* (int)(x/y);
   }
 else
   {
     result = 0;
    }

 return result;
}

int main(const int argc, char** argv) {
  // Constant
  double pi = 3.14159;

  // Data
  std::size_t siglen = 4096;
  glth::analog_signal sig(siglen);
  int fftlen = 2048;
  std::complex<double> *lacf = new std::complex<double>[siglen];
  std::complex<double> *out = new std::complex<double>[fftlen];
  std::complex<double> **wvd = new std::complex<double>*[siglen];
  for(std::size_t i = 0; i < siglen; i++) {
    wvd[i] = new std::complex<double>[fftlen];
  }

  // FFTW plan
  fftw_plan p = fftw_plan_dft_1d(fftlen, 
				 reinterpret_cast<fftw_complex*>(lacf), 
				 reinterpret_cast<fftw_complex*>(out), 
				 FFTW_FORWARD, 
				 FFTW_ESTIMATE);

  // By default we have succeeded.
  glth_const::exit res = glth_const::success;

  // Make two signals and XWVD them.
  float freq_0 = 0.1;
  float chirp_rate = 0.0005;

  for(int idx = 0; idx < siglen; idx++) {
    double phase = freq_0*idx + chirp_rate*idx*idx;
    sig[idx] = std::complex<double>(cos(2*pi*phase),sin(2*pi*phase));
  }

  std::ofstream sigfile("glth_sig_out.dat");
  for(std::size_t i = 0; i < siglen; i++) {
    sigfile << i << "," << sig[i] << std::endl;
  }
  sigfile.close();

  // Now WVD it.
  int tau, taumax;

  for(int t = 0; t < siglen; t++) {

    // How large in tau should we go?
    taumax = (t < (siglen - t - 1)) ? t : (siglen - t - 1);
    taumax = (taumax < (fftlen/2 - 1)) ? taumax : (fftlen/2 - 1);

    for(tau = -taumax; tau <= taumax; tau++) {
      int row = irem(fftlen + tau, fftlen);
      lacf[row] = sig[t + tau]*std::conj(sig[t - tau]);
    }

    tau = floor(fftlen/2);
        if((t <= (siglen - tau - 1)) && (t >= tau)) {
	  lacf[t] =0.5*(sig[t + tau]*std::conj(sig[t - tau]) + 
			sig[t - tau]*std::conj(sig[t + tau]));
    }

    fftw_execute(p);

    // write to WVD
    for(std::size_t i = 0; i < fftlen; i++) {
      wvd[t][i] = out[i];
    }
  }

  // Now dump to disk in the form of total data and frames.
  std::ofstream datafile("glth_out.dat");
  for(std::size_t i = 0; i < siglen; i++) {
    for(std::size_t j = 0; j < fftlen; j++) {
      datafile << i << "," << j << "," << std::real(wvd[i][j]) << "," << std::imag(wvd[i][j]) << std::endl;
    }
  }
  datafile.close();

  for(std::size_t i = 0; i < siglen; i++) {
    char buf[18];
    sprintf(buf,"frame/out%04d.dat",(int)i);
    std::ofstream data(buf);
    for(std::size_t j = 0; j < fftlen; j++) {
      data << i << "," << j << "," << std::real(wvd[i][j]) << "," << std::imag(wvd[i][j]) << std::endl;
    }
    data.close();
  }

  return res;
}
