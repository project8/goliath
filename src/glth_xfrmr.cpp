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
			       FFTW_MEASURE);
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

void glth::glth_xfrmr::wvd(glth::signal tgt, std::complex<double>** out) 
{
  int tau, taumax;
  int siglen = tgt.size();

  glth::signal tgt_star(tgt.size());
  for(int i = 0; i < siglen; i++) {
    tgt_star[i] = std::conj(tgt[i]);
  }

  //#pragma omp for private(tau)
  for(int t = 0; t < siglen; t++) {
    
    // How large in tau should we go?
    taumax = (t < (siglen - t - 1)) ? t : (siglen - t - 1);
    taumax = (taumax < (_freq_res/2 - 1)) ? taumax : (_freq_res/2 - 1);

    for(tau = -taumax; tau <= taumax; tau++) {
      int row = irem(_freq_res + tau, _freq_res);
      _in[row] = (tgt[t + tau])*(tgt_star[t - tau]);
    }

    tau = floor(_freq_res/2);
    if((t <= (siglen - tau - 1)) && (t >= tau)) {
      _in[t] =0.5*(tgt[t + tau])*(tgt_star[t - tau]) + 
	(tgt[t - tau])*(tgt_star[t + tau]);
    }

    fftw_execute_dft(_fwd_plan,
		     reinterpret_cast<fftw_complex*>(_in), 
		     reinterpret_cast<fftw_complex*>(out[t]));
  }
}
