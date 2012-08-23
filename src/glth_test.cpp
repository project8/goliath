#include <iostream>
#include <fstream>
#include <cmath>
#include <tr1/random>
#include "glth_xfrmr.hpp"
 
int main(const int argc, char** argv) {
  // Random number generation
  std::tr1::mt19937 prng(0);
  std::tr1::normal_distribution<double> normal;
  std::tr1::variate_generator<std::tr1::mt19937,std::tr1::normal_distribution<double>> randn(prng,normal);
  
  // Constant
  double pi = 3.14159;

  // Data
  std::size_t siglen = 256;
  glth::analog_signal sig(siglen);
  int fftlen = 128;
  glth::glth_xfrmr xfm(siglen, siglen, fftlen);

  std::complex<double> **wvd = new std::complex<double>*[siglen];
  for(std::size_t i = 0; i < siglen; i++) {
    wvd[i] = new std::complex<double>[fftlen];
  }


  // By default we have succeeded.
  glth_const::exit res = glth_const::success;

  // Make two signals and XWVD them.
  float freq_0 = 30e6;
  float pitch_angle = 90;
  float power_loss = 1.0e-15;  // Watts
  float energy_0 = 18e3; // keV
  float zmax = 1.0e-3; // meters
  float warble = 10e6; // axial frequency
  float phase_v = 299792458; // phase velocity, approx at c for now
  float gamma_0 = 1 + energy_0/511;
  float chirp_rate = power_loss/(gamma_0 * 8.18e-14);
  float sample_rate = 250e6;
  float t = 0;

  for(int idx = 0; idx < siglen; idx++) {
    t = idx/sample_rate;
    double phase = freq_0*t*
      ((1 + 0.5*cos(pitch_angle)*cos(pitch_angle)/(sin(pitch_angle)*sin(pitch_angle))) + 
       chirp_rate*t +
       zmax*warble/phase_v*cos(warble*t));
    std::complex<double> noise = std::complex<double>(randn(),randn());
    sig[idx] = std::complex<double>(cos(2*pi*phase),sin(2*pi*phase)) + 0.3*noise;
  }

   std::ofstream sigfile("glth_sig_out.dat");
   for(std::size_t i = 0; i < siglen; i++) {
       sigfile << i << "," << sig[i] << std::endl;
   }
   sigfile.close();

   xfm.wvd(sig, wvd);

  //  Now dump to disk in the form of total data and frames.
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
