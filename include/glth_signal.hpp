#ifndef __glth_sig_hpp
#define __glth_sig_hpp

#include "glth_const.hpp"
#include "glth_types.hpp"
#include "glth_px1500.hpp"
#include "fftw3.h"

#include <cstring>

namespace glth
{

    /*
     * goliath considers a 'signal' to be a complex valued vector.
     */
    //typedef std::vector<std::complex<double> > signal;
    class signal
    {
        private:
            signal(); // not implemented, calling will cause compiler error

        public:
            signal( const size_t& a_size );
            ~signal();

            void copy_all( const signal& a_signal );
            void zero_all();

            const size_t& size() const;

            fftw_complex* data();
            const fftw_complex* data() const;

            operator fftw_complex*();
            operator const fftw_complex*() const;

            fftw_complex& operator[]( const size_t& an_index );
            const fftw_complex& operator[]( const size_t& an_index ) const;

        private:
            size_t _size;
            fftw_complex* _data;
    };

    inline void signal::copy_all( const signal& a_signal )
    {
        memcpy( _data, a_signal.data(), _size * sizeof( fftw_complex ) );
        return;
    }
    inline void signal::zero_all()
    {
        memset( _data, 0, _size * sizeof( fftw_complex ) );
        return;
    }

    inline const size_t& signal::size() const
    {
        return _size;
    }

    inline fftw_complex& signal::operator[]( const size_t& an_index )
    {
        return _data[an_index];
    }
    inline const fftw_complex& signal::operator[]( const size_t& an_index ) const
    {
        return _data[an_index];
    }

    inline signal::operator fftw_complex*()
    {
        return _data;
    }
    inline signal::operator const fftw_complex*() const
    {
        return _data;
    }

    inline fftw_complex* signal::data()
    {
        return _data;
    }
    inline const fftw_complex* signal::data() const
    {
        return _data;
    }

} // namespace glth_signal

#endif // __glth_sig_hpp
