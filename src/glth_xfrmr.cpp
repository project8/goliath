#include "glth_xfrmr.hpp"

#include <cstring>

glth::glth_xfrmr::glth_xfrmr( std::size_t size, std::size_t freq_bins, std::size_t time_bins ) :
    _size( size ),
    _freq_bins( freq_bins ),
    _time_bins( time_bins ),
    _aa_in( glth::signal( _size ) ),
    _aa_out( glth::signal( _size ) ),
    _aa_forward_plan( fftw_plan_dft_1d( _size, _aa_in, _aa_out, FFTW_FORWARD, FFTW_MEASURE ) ),
    _aa_reverse_plan( fftw_plan_dft_1d( _size, _aa_out, _aa_in, FFTW_FORWARD, FFTW_MEASURE ) ),
    _wvd_in( glth::signal( _freq_bins ) ),
    _wvd_out( glth::signal( _freq_bins ) ),
    _vwd_plan( fftw_plan_dft_1d( _freq_bins, _wvd_in, _wvd_out, FFTW_FORWARD, FFTW_MEASURE ) )
{
}

void glth::glth_xfrmr::aa( const glth::signal& in, glth::signal& out )
{
    //size of arrays better both be _in_size...
    register size_t t_index;

    //copy in signal to buffer
    _aa_in.copy_from( in );

    //compute forward transform
    fftw_execute( _aa_forward_plan );

    //filter
    for( t_index = 1; t_index < _size / 2; t_index++ )
    {
        _aa_out[t_index][0] *= 2;
        _aa_out[t_index][1] *= 2;
    }
    for( t_index = _size / 2 + 1; t_index < _size; t_index++ )
    {
        _aa_out[t_index][0] = 0.;
        _aa_in[t_index][1] = 0.;
    }

    //compute reverse transform
    fftw_execute( _aa_reverse_plan );

    //normalize
    for( t_index = 1; t_index < _size / 2; t_index++ )
    {
        _aa_out[t_index][0] /= _size;
        _aa_out[t_index][1] /= _size;
    }

    //copy buffer to out signal
    out.copy_from( _aa_out );

    return;
}

void glth::glth_xfrmr::wvd( const glth::signal& tgt, glth::tfr_data& out )
{
    return this->xwvd( tgt, tgt, out );
}

void glth::glth_xfrmr::xwvd( const glth::signal& tgt1, const glth::signal& tgt2, glth::tfr_data& out )
{
    size_t time_stride = _size / _time_bins;
    size_t time_index = 0;

    register double t1_real;
    register double t1_imag;
    register double t2_real;
    register double t2_imag;

    for( size_t t = 0; t < _time_bins; t++ )
    {
        for( size_t tau = 0; tau < _freq_bins; tau++ )
        {
            t1_real = tgt1[time_index + tau][0];
            t1_imag = tgt1[time_index + tau][1];
            t2_real = tgt1[time_index + _freq_bins - 1 - tau][0];
            t2_imag = tgt1[time_index + _freq_bins - 1 - tau][0];
            _wvd_in[tau][0] = t1_real * t2_real + t1_imag * t2_imag;
            _wvd_in[tau][1] = t1_imag * t2_real - t1_real * t2_imag;
        }
        fftw_execute( _vwd_plan );
        memcpy( out[t], _wvd_out.data(), _freq_bins * sizeof(std::complex< double >) );
        time_index += time_stride;
    }
}
