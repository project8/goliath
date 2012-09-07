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

cv::Vec3b JetColour(double value,double vmax,double vmin)
{
  double v = value - vmin;
  double scale = (vmax - vmin)/4.0;
  unsigned char mid = 144;
  unsigned char red, blue, green;

  if( v < scale ) {
    blue = mid+(char)((255-mid)*v/vmax);
    red = green = 0;
  }
  else if( v < 2*scale ) {
    red=0;
    green=(unsigned char)(255*(v-scale)/scale);
    blue=255;
  }
  else if( v < 3*scale ) {
    red=(unsigned char)(255*(v-2*scale)/scale);
    green=255;
    blue=255-red;
  }
  else if( v < vmax ) {
    red=255;
    green=(unsigned char)(255-255*(v-3*scale)/scale);
    blue=0;
  }
  else {
    red = blue = green = 0;
  }

  return(cv::Vec3b(blue,green,red));
}


using namespace glth;

int main( const int argc, char** argv )
{
    // OK now parse the arguments passed by the user and
    // verify that they are sensible.
    env const* env = env::env_from_args( argc, argv );

    if( env::verify( *env ) != glth_const::env_success )
    {
        std::cout << "[error] failed to create environment" << std::endl;
        return glth_const::exit_env_failed;
    }

    else
    {
        // Our pointer to the IO object which gets our data for us
        io* io = io::open_file( env->get_in_filename() );

        // IO being a valid pointer means we're in business.
        if( io )
        {
            std::size_t sample_count = (io->get_record_length());
            double sample_time_res = sample_count / (1.e6 * io->get_record_frequency());
            double sample_freq_res = (1.e6 * io->get_record_frequency()) / 2.;
            std::cout << "[normal] file time width is <" << sample_time_res << ">" << endl;
            std::cout << "[normal] file frequency width is <" << sample_freq_res << ">" << endl;

            // calculate number of time bins
            std::size_t time_bins = env->get_time_res();
            cout << "[normal] using <" << time_bins << "> time bins for a resolution of <" << sample_time_res / time_bins << ">" << std::endl;

            // calculate number of frequency bins
            std::size_t freq_bins = env->get_freq_res();
            cout << "[normal] using <" << freq_bins << "> frequency bins for a resolution of <" << sample_freq_res / freq_bins << ">" << std::endl;

            // allocate channel signals
            std::cout << "[normal] allocating channel signals" << std::endl;
            glth::signal ch1( sample_count );
            glth::signal ch1_aa( sample_count );
            glth::signal ch2( sample_count );
            glth::signal ch2_aa( sample_count );

            // allocate target data block
            std::cout << "[normal] allocating target data" << endl;
            tfr_data tfr( time_bins, freq_bins );

            // allocate transformer
            std::cout << "[normal] allocating transformer" << endl;
            glth::glth_xfrmr xfm( sample_count, freq_bins, time_bins );

            glth_const::io_result proc_res;
            std::size_t evt = 0;
            while( (proc_res = io->populate( ch1, ch2 )) == glth_const::io_read_ok )
            {
                std::cout << "[normal] processing event <" << evt << ">..." << std::endl;

                // Calculate the analytic associate signal for each channel
                std::cout << "[normal] calculating analytic associate..." << std::endl;
                xfm.aa( ch1, ch1_aa );
                xfm.aa( ch2, ch2_aa );

                // Calculate the cross wigner ville distribution for the current event
                std::cout << "[normal] calculating wigner-ville distribution..." << std::endl;
                xfm.xwvd( ch1_aa, ch2_aa, tfr );
            }

            double min = 1.e300;
            double max = -1.e300;
            double norm;
            for( int i = 0; i < time_bins; i++ )
            {
                for( int j = 0; j < freq_bins; j++ )
                {
                    norm = log10( std::norm( tfr[i][j] ) );
                    if( norm > max ) max = norm;
                    if( norm < min ) min = norm;
                }
            }

            cv::Mat output(freq_bins, time_bins, CV_8UC3, cv::Scalar(0,0,0));
            for( int i = 0; i < time_bins; i++ )
            {
                for( int j = 0; j < freq_bins; j++ )
                {
                    output.at<cv::Vec3b>( j, i ) =  JetColour(log10( std::norm(tfr[i][j]) ),min,max);
                }
            }

            cv::imwrite("glth_out.png",output);
        }
    }

    return 0;
}
