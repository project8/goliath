#include <iostream>
#include <fstream>
#include <cmath>

#include "glth_io.hpp"
#include "glth_env.hpp"
#include "glth_signal.hpp"
#include "glth_tfr_data.hpp"
#include "glth_xfrmr.hpp"
#include "glth_candidate.hpp"
#include "glth_color.hpp"
#include "glth_discrim.hpp"

#include <cv.h>
#include <highgui.h>

template< typename T >
T min( T arg1, T arg2 )
{
    T res = (arg1 < arg2) ? arg1 : arg2;
    return res;
}

std::size_t next_po2( double arg )
{
    int res = pow( 2, ceil( log2( arg ) ) );
    return (std::size_t) res;
}

cv::Vec3b JetColour( double v, double vmax )
{
    double scale = vmax / 4.0;
    unsigned char mid = 144;
    unsigned char red, blue, green;

    if( v < scale )
    {
        blue = mid + (char) ((255 - mid) * v / vmax);
        red = green = 0;
    }
    else if( v < 2 * scale )
    {
        red = 0;
        green = (unsigned char) (255 * (v - scale) / scale);
        blue = 255;
    }
    else if( v < 3 * scale )
    {
        red = (unsigned char) (255 * (v - 2 * scale) / scale);
        green = 255;
        blue = 255 - red;
    }
    else if( v < vmax )
    {
        red = 255;
        green = (unsigned char) (255 - 255 * (v - 3 * scale) / scale);
        blue = 0;
    }
    else
    {
        red = blue = green = 0;
    }

    return (cv::Vec3b( blue, green, red ));
}

using namespace glth;

int main( const int argc, char** argv )
{
    // OK now parse the arguments passed by the user and
    // verify that they are sensible.
    env const* env = env::env_from_args( argc, argv );

    if( env::verify( *env ) != glth_const::env_success )
    {
        std::cout << "  **failed to create valid environment!" << std::endl;
        return glth_const::exit_env_failed;
    }

    else
    {
        // Our pointer to the IO object which gets our data for us
        io* io = io::open_file( env->get_in_filename() );

        // Signals!
        std::size_t record_len = (io->get_record_length());
        std::cout << "**Record length: " << record_len << std::endl;
        std::cout << "**Allocating channel signals" << std::endl;

        glth::signal ch1( record_len ), ch2( record_len );

        // IO being a valid pointer means we're in business.
        if( io )
        {
            // An event counter
            std::size_t evt = 0;

            // Create a TFR data object to hold the result of our
            // WVD.  This object will get reused for analysis on each event.
            // But how do we know how many frequency bins we need?  We use the
            // Nyquist rate from the header and our desired frequency resolution,
            // which in this case is hard coded at 30kHz.  We find the next highest
            // power of two and that's our binning.
            double nyquist_f = (io->get_monarch_ptr()->GetHeader()->GetAcqRate()) / 2.0;
            std::cout << "*Nyquist frequency is " << nyquist_f << "MHz" << std::endl;

            double freq_res = 30.0e3;
            std::size_t nbins = next_po2( nyquist_f * 1.0e6 / freq_res );
            if( nbins > 2048 )
            {
                std::cout << "**>nbins too large! (" << nbins << ")" << ", adjusting to 128" << std::endl;
                nbins = 1024;
            }
            std::cout << "**Using " << nbins << " FFT frequency bins" << std::endl;
            freq_res = nyquist_f * 1.0e6 / (double) nbins;
            std::cout << "**Actual frequency resolution is " << floor( freq_res ) << "Hz" << std::endl;

            // We need a transformer
            glth::glth_xfrmr xfm( record_len, nbins );

            std::cout << "**Allocating " << record_len << "x" << nbins << " TFR data block" << std::endl;
            tfr_data tfr( record_len - nbins, nbins );

            // At this point we're ready to rock.  We grab records, convert them to
            // analog signals, and then XWVD the two channels.
            glth_const::io_result proc_res;
            while( (proc_res = io->populate( ch1, ch2 )) == glth_const::io_read_ok )
            {
                std::cout << "*Processing event #" << evt << std::endl;

                // Calculate the analytic associate signal for each channel
                std::cout << "**Calculating AA..." << std::endl;
                xfm.aa( ch1, ch1 );
                xfm.aa( ch2, ch2 );

                // Calculate the cross wigner ville distribution for the current event
                std::cout << "**Calculating XWVD..." << std::endl;
                xfm.xwvd( ch1, ch2, &tfr );

            }

            std::cout << "**Finding extrema..." << std::endl;

            double min, max;
            for( int i = 0; i < record_len - nbins; i++ )
            {
                for( int j = 0; j < nbins; j++ )
                {
                    //wvd[i][j] = log10(wvd[i][j]);
                    if( std::norm( tfr[i][j] ) > max )
                        max = std::norm( tfr[i][j] );
                    else if( std::norm( tfr[i][j] ) < min ) min = std::norm( tfr[i][j] );
                }
            }
            std::cout << " <max was: " << max << ">" << std::endl;

            std::cout << "**Filling matrix..." << std::endl;

            cv::Mat out_matrix( record_len - 1 - nbins, nbins, CV_8UC3, cv::Scalar( 0, 0, 0 ) );
            for( int t = 0; t < record_len - nbins; t++ )
            {
                for( int f = 0; f < nbins; f++ )
                {
                    out_matrix.at < cv::Vec3b > ((record_len - 1 - nbins) - f, t) = JetColour( std::norm( tfr[t][f] ), max );
                }
            }

            std::cout << "**Writing..." << std::endl;

            cv::imwrite( "glth_out.png", out_matrix );

            std::cout << "**Peace!" << std::endl;
        }
    }

    return 0;
}
