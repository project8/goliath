#ifndef __glth_sig_hpp
#define __glth_sig_hpp

#include "glth_const.hpp"
#include "glth_types.hpp"
#include "glth_px1500.hpp"

#include <cstring>

namespace glth
{

    /*
     * goliath considers a 'signal' to be a complex valued vector.
     */

    typedef double cplx[2];

    class signal
    {
        private:
            signal(); // not implemented, calling will cause compiler error

        public:
            signal( const size_t& a_size );
            ~signal();

            void copy_to( signal& a_signal );
            void copy_from( const signal& a_signal );
            void zero_all();

            const size_t& size() const;

            // index operators
            cplx& operator[]( const size_t& an_index );
            const cplx& operator[]( const size_t& an_index ) const;

            // access as double[2]/fftw_complex
            cplx* data()
                { return _data; }
            const cplx* data() const
                { return _data; }
            operator cplx*()
                { return _data; }
            operator const cplx*() const
                { return _data; }

        private:
            size_t _size;
            cplx* _data;
    };

    inline void signal::copy_to( signal& a_signal )
    {
        memcpy( a_signal._data, _data, _size * sizeof( cplx ) );
        return;
    }
    inline void signal::copy_from( const signal& a_signal )
    {
        memcpy( _data, a_signal._data, _size * sizeof( cplx ) );
        return;
    }
    inline void signal::zero_all()
    {
        memset( _data, 0, _size * sizeof( cplx ) );
        return;
    }

    inline const size_t& signal::size() const
    {
        return _size;
    }

    inline cplx& signal::operator[]( const size_t& an_index )
    {
        return _data[an_index];
    }
    inline const cplx& signal::operator[]( const size_t& an_index ) const
    {
        return _data[an_index];
    }


} // namespace glth_signal

#endif // __glth_sig_hpp
