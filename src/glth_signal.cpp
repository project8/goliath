#include "glth_signal.hpp"
#include "fftw3.h"

namespace glth
{
    signal::signal( const size_t& a_size ) :
        _size( a_size ),
        _data( (cplx*) fftw_malloc( a_size * sizeof(cplx) ) )
    {
    }
    signal::~signal()
    {
        fftw_free( _data );
    }
}
