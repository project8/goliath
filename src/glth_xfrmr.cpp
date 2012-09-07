#include "glth_xfrmr.hpp"

#include <cstring>

glth::glth_xfrmr::glth_xfrmr( std::size_t size, std::size_t resolution ) :
    _size( size ),
    _freq_res( resolution ),
    _aa_in( glth::signal( _size ) ),
    _aa_out( glth::signal( _size ) ),
    _wvd_in( glth::signal( _freq_res ) ),
    _wvd_out( glth::signal( _freq_res ) ),
    _vwd_plan( fftw_plan_dft_1d( _freq_res, _wvd_in, _wvd_out, FFTW_FORWARD, FFTW_MEASURE ) ),
    _aa_forward_plan( fftw_plan_dft_1d( _size, _aa_in, _aa_out, FFTW_FORWARD, FFTW_MEASURE ) ),
    _aa_reverse_plan( fftw_plan_dft_1d( _size, _aa_out, _aa_in, FFTW_FORWARD, FFTW_MEASURE ) )
{
}

int glth::glth_xfrmr::irem( double x, double y )
{
    int result;

    if( y != 0 )
    {
        result = x - y * (int) (x / y);
    }
    else
    {
        result = 0;
    }

    return result;
}

void glth::glth_xfrmr::aa( const glth::signal& in, glth::signal& out )
{
    //size of arrays better both be _in_size...
    register size_t t_index;

    //copy in signal to buffer
    _aa_in.copy_all( in );

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
    out.copy_all( _aa_out );

    return;
}

void glth::glth_xfrmr::wvd( const glth::signal& tgt, glth::tfr_data* out )
{
    return this->xwvd( tgt, tgt, out );
}

void glth::glth_xfrmr::xwvd( const glth::signal& tgt1, const glth::signal& tgt2, glth::tfr_data* out )
{
    size_t tau;
    for( size_t t = 0; t < _size - _freq_res; t++ )
    {
        _wvd_in.zero_all();
        for( tau = 0; tau < _freq_res; tau++ )
        {
            _wvd_in[tau][0] = tgt1[t + tau][0] * tgt2[t + _freq_res - 1 - tau][0] + tgt1[tau][1] * tgt2[t + _freq_res - 1 - tau][1];
        }
        fftw_execute( _vwd_plan );
        memcpy( (*out)[t], _wvd_out.data(), _wvd_out.size() * sizeof( fftw_complex ) );
    }
}
