#include "glth_xfrmr.hpp"

#include <cstring>
#include <cstdio>

#include <fstream>
#include <iostream>
using std::cout;
using std::endl;

glth::glth_xfrmr::glth_xfrmr( std::size_t size, std::size_t freq_bins, std::size_t time_bins ) :
    _size( size ),
    _freq_bins( freq_bins ),
    _time_bins( time_bins ),
    _aa_sig_buf( glth::signal( _size ) ),
    _aa_xfm_buf( glth::signal( _size ) ),
    _aa_forward_plan( NULL ),
    _aa_reverse_plan( NULL ),
    _wvd_in( glth::signal( _freq_bins ) ),
    _wvd_out( glth::signal( _freq_bins ) ),
    _wvd_plan( NULL )
{
    bool t_wisdom_flag = false;
    FILE* t_wisdom_input_file = NULL;
    FILE* t_wisdom_output_file = NULL;

    t_wisdom_input_file = fopen("./glth_wsdm.fftw3","r");
    if( t_wisdom_input_file != NULL )
    {
        if( fftw_import_wisdom_from_file( t_wisdom_input_file ) != 0 )
        {
            t_wisdom_flag = true;
        }
        fclose( t_wisdom_input_file );
    }

    _aa_forward_plan = fftw_plan_dft_1d( _size, _aa_sig_buf, _aa_xfm_buf, FFTW_FORWARD, FFTW_MEASURE );
    _aa_reverse_plan = fftw_plan_dft_1d( _size, _aa_xfm_buf, _aa_sig_buf, FFTW_BACKWARD, FFTW_MEASURE );
    _wvd_plan = fftw_plan_dft_1d( _freq_bins, _wvd_in, _wvd_out, FFTW_FORWARD, FFTW_MEASURE );

    if( t_wisdom_flag == false )
    {
        t_wisdom_output_file = fopen("./glth_wsdm.fftw3","w");
        if( t_wisdom_output_file != NULL )
        {
            fftw_export_wisdom_to_file( t_wisdom_output_file );
            fclose( t_wisdom_output_file );
        }
    }
}
glth::glth_xfrmr::~glth_xfrmr()
{
    fftw_destroy_plan( _aa_forward_plan );
    fftw_destroy_plan( _aa_reverse_plan );
    fftw_destroy_plan( _wvd_plan );
}

void glth::glth_xfrmr::aa( const glth::signal& in, glth::signal& out )
{
    //size of arrays better both be _in_size...
    register size_t t_index;

    //copy in signal to buffer
    _aa_sig_buf.copy_from( in );

    //compute forward transform
    fftw_execute( _aa_forward_plan );

    //filter
    for( t_index = 1; t_index < _size / 2; t_index++ )
    {
        _aa_xfm_buf[t_index][0] *= 2;
        _aa_xfm_buf[t_index][1] *= 2;
    }
    for( t_index = _size / 2 + 1; t_index < _size; t_index++ )
    {
        _aa_xfm_buf[t_index][0] = 0.;
        _aa_xfm_buf[t_index][1] = 0.;
    }

    //compute reverse transform
    fftw_execute( _aa_reverse_plan );

    //normalize
    for( t_index = 0; t_index < _size; t_index++ )
    {
        _aa_sig_buf[t_index][0] /= _size;
        _aa_sig_buf[t_index][1] /= _size;
    }

    //copy buffer to out signal
    _aa_sig_buf.copy_to( out );

    return;
}

void glth::glth_xfrmr::wvd( const glth::signal& in, glth::signal& out, size_t offset )
{
    return this->xwvd( in, in, out, offset );
}

void glth::glth_xfrmr::xwvd( const glth::signal& in_1, const glth::signal& in_2, glth::signal& out, size_t offset )
{
    register double t1_real;
    register double t1_imag;
    register double t2_real;
    register double t2_imag;
    register size_t tau_index_minus;
    register size_t tau_index_plus;

    //compute the correlation between in_1 and in_2
    tau_index_minus = 0;
    tau_index_plus = _freq_bins - 1;
    for( size_t j = 0; j < _freq_bins; j++ )
    {
        t1_real = in_1[offset + tau_index_minus][0];
        t1_imag = in_1[offset + tau_index_minus][1];
        t2_real = in_2[offset + tau_index_plus][0];
        t2_imag = in_2[offset + tau_index_plus][1];

        _wvd_in[j][0] = t1_real * t2_real + t1_imag * t2_imag;
        _wvd_in[j][1] = t1_imag * t2_real - t1_real * t2_imag;

        tau_index_minus++;
        tau_index_plus--;
    }

    //execute the fft
    fftw_execute( _wvd_plan );

    //copy the output
    _wvd_out.copy_to( out );
    return;
}

void xwvd_slice( const glth::signal& tgt1, const glth::signal& tgt2, glth::signal& out, const int instant )
{
    /* no-op */
}
