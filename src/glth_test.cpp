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
  float pi = 3.14159;

  // Data
  std::size_t siglen = 256;
  glth::analog_signal sig(siglen);
  int fftlen = 128;
  double *lacf = new double[siglen];
  double *out = new double[fftlen];
  double **wvd = new double*[siglen];
  for(std::size_t i = 0; i < siglen; i++) {
    wvd[i] = new double[fftlen];
  }

  // FFTW plan
  fftw_plan p = fftw_plan_r2r_1d(fftlen, lacf, out, 
				 FFTW_R2HC, FFTW_ESTIMATE);

  // By default we have succeeded.
  glth_const::exit res = glth_const::success;

  // Make two signals and XWVD them.
  float freq_0 = 0.1;
  float chirp_rate = 0.0001;

  for(std::size_t i = 0; i < siglen; i++) {
    sig[i] = sin(2*pi*(freq_0*i + chirp_rate*i*i));
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
      lacf[row] = sig[t + tau]*sig[t - tau];
    }

    tau = floor(fftlen/2);
    if((t <= (siglen - tau - 1)) && (t >= tau)) {
      lacf[t] = sig[t + tau]*sig[t - tau];
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
      datafile << i << "," << j << "," << wvd[i][j] << std::endl;
    }
  }
  datafile.close();

  for(std::size_t i = 0; i < siglen; i++) {
    char buf[18];
    sprintf(buf,"frame/out%04d.dat",(int)i);
    std::ofstream data(buf);
    for(std::size_t j = 0; j < fftlen; j++) {
      data << i << "," << j << "," << wvd[i][j] << std::endl;
    }
    data.close();
  }

  return res;
}
