#include "glth_tfr_data.hpp"

glth::tfr_data::tfr_data( std::size_t nt, std::size_t nf ) :
    _ntime( nt ),
    _nfreq( nf )
{
    _data = new std::complex< double >*[_ntime];
    for( std::size_t idx = 0; idx < _ntime; idx++ )
    {
        _data[idx] = new std::complex< double >[_nfreq];
    }
}
glth::tfr_data::~tfr_data()
{
    for( std::size_t idx = 0; idx < _ntime; idx++ )
    {
        delete[] _data[idx];
    }
    delete[] _data;
}

std::size_t glth::tfr_data::get_ntime()
{
    return (this->_ntime);
}

std::size_t glth::tfr_data::get_nfreq()
{
    return (this->_nfreq);
}

std::complex< double >* glth::tfr_data::operator[]( std::size_t colidx )
{
    if( colidx >= (this->_ntime) )
    {
        return NULL;
    }
    else
        return (this->_data[colidx]);
}
