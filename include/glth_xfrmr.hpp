#ifndef __glth_xfrmr_hpp
#define __glth_xfrmr_hpp

#include "glth_signal.hpp"
#include "glth_tfr_data.hpp"
extern "C"
{
#include "fftw3.h"
}

namespace glth
{

    class glth_xfrmr
    {

        private:
            std::size_t _size;
            std::size_t _freq_bins;
            std::size_t _time_bins;
            glth::signal _aa_sig_buf;
            glth::signal _aa_xfm_buf;
            glth::signal _wvd_in;
            glth::signal _wvd_out;
            fftw_plan _wvd_plan;
            fftw_plan _aa_forward_plan;
            fftw_plan _aa_reverse_plan;

        public:
            glth_xfrmr( std::size_t size, std::size_t freq_bins, std::size_t time_bins );
            ~glth_xfrmr();

            // Calculate the analytic associate of the signal
            void aa( const glth::signal& in, glth::signal& out );

            // Calculate the wigner-ville distribution for an input signal.
            // Equivalent to xwvd(tgt, tgt, out).
            void wvd( const glth::signal& in, glth::signal& out, size_t offset );

            // Calculate the cross wigner-ville distribution for two signals.
            void xwvd( const glth::signal& in_1, const glth::signal& in_2, glth::signal& out, size_t offset );
    };

}
;
// namespace glth

#endif // __glth_xfrmr_hpp
