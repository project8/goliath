#include "glth_xfrmr.hpp"

glth::glth_xfrmr::glth_xfrmr(std::size_t in_size,
		 std::size_t out_size,
		 std::size_t resolution) 
  : _in_size(in_size), 
    _out_size(out_size), 
    _freq_res(resolution),
    _in(new std::complex<double>[in_size]),
    _out(new std::complex<double>[out_size])
{
  _fwd_plan = fftw_plan_dft_1d(resolution,
			       reinterpret_cast<fftw_complex*>(_in),
			       reinterpret_cast<fftw_complex*>(_out),
			       FFTW_FORWARD,
			       FFTW_PATIENT);
}

int glth::glth_xfrmr::irem( double x, double y)
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

void glth::glth_xfrmr::wvd(glth::signal tgt, 
			   glth::tfr_data* out) 
{
  return this->xwvd(tgt,tgt,out);
}

void glth::glth_xfrmr::xwvd(glth::signal tgt1, 
			    glth::signal tgt2,
			    glth::tfr_data* out) 
{
  int tau, taumax;
  int siglen = tgt1.size();

  glth::signal tgt2_star(tgt2.size());
  for(int i = 0; i < siglen; i++) {
    tgt2_star[i] = std::conj(tgt2[i]);
  }

  //#pragma omp for private(tau)
  for(int t = 0; t < siglen; t++) {
    
    // How large in tau should we go?
    taumax = (t < (siglen - t - 1)) ? t : (siglen - t - 1);
    taumax = (taumax < (_freq_res/2 - 1)) ? taumax : (_freq_res/2 - 1);

    for(tau = -taumax; tau <= taumax; tau++) {
      int row = irem(_freq_res + tau, _freq_res);
      _in[row] = (tgt1[tau])*(tgt2_star[t - tau]);
    }

    tau = floor(_freq_res/2);
    if((t <= (siglen - tau - 1)) && (t >= tau)) {
      _in[t] =0.5*(tgt1[tau])*(tgt2_star[t - tau]) + 
	(tgt1[tau])*(tgt2_star[t + tau]);
    }

    fftw_execute_dft(_fwd_plan,
		     reinterpret_cast<fftw_complex*>(_in), 
		     reinterpret_cast<fftw_complex*>((*out)[t]));
  }
}

void glth::glth_xfrmr::xwvd_slice(glth::signal tgt1, 
				  glth::signal tgt2,
				  glth::tfr_data* out,
				  int t) 
{
  int tau, taumax;
  int siglen = tgt1.size();

  // Blank the input array.
  std::memset(_in, 0, _in_size*sizeof(std::complex<double>));

  // Precompute the complex conjugate of the second signal.
  glth::signal tgt2_star(tgt2.size());
  for(int i = 0; i < siglen; i++) {
    tgt2_star[i] = std::conj(tgt2[i]);
  }

  // Calculate the lagged correlation between the two
  // signals at time t.
  
  // How large in tau should we go?
  taumax = (t < (siglen - t - 1)) ? t : (siglen - t - 1);
  taumax = (taumax < (_freq_res/2 - 1)) ? taumax : (_freq_res/2 - 1);
    
  for(tau = -taumax; tau <= taumax; tau++) {
    int row = irem(_freq_res + tau, _freq_res);
    _in[row] = (tgt1[tau])*(tgt2_star[t - tau]);
  }
  
  tau = floor(_freq_res/2);
  if((t <= (siglen - tau - 1)) && (t >= tau)) {
    _in[t] =0.5*(tgt1[tau])*(tgt2_star[t - tau]) + 
      (tgt1[tau])*(tgt2_star[t + tau]);
  }
  
  fftw_execute_dft(_fwd_plan,
		   reinterpret_cast<fftw_complex*>(_in), 
		   reinterpret_cast<fftw_complex*>((*out)[t])); 
}
