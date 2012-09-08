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

  signal& signal::operator=(const signal& tgt) 
  {
    // protect against self-assignment
    if(this != (&tgt)) {
      // get rid of old stuff
      fftw_free(this->_data);
      // OK now get new memory, and...
      this->_size = tgt.size();
      this->_data = (cplx*) fftw_malloc( this->_size * sizeof(cplx) );
      // now copy from the target
      this->copy_from(tgt);
    }
    return *this;
  }

  signal::signal( const signal& tgt ) :
    _size(tgt.size()),
    _data( (cplx*) fftw_malloc(tgt.size() * sizeof(cplx)) ) 
  {         
    memcpy( _data, tgt._data, _size * sizeof( cplx ) ); 
  }
  

}
