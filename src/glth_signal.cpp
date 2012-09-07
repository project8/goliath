#include "glth_signal.hpp"

namespace glth
{
    signal::signal( const size_t& a_size ) :
        _size( a_size ),
        _data( (fftw_complex*) fftw_malloc( a_size * sizeof(fftw_complex) ) )
    {
    }
    signal::~signal()
    {
        fftw_free( _data );
    }
}
