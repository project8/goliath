#include <iostream>
#include <fstream>
#include <cmath>

#include <cv.h>
#include <highgui.h>

#include "glth_io.hpp"
#include "glth_env.hpp"
#include "glth_signal.hpp"
#include "glth_tfr_data.hpp"
#include "glth_xfrmr.hpp"
#include "glth_candidate.hpp"
#include "glth_color.hpp"
#include "glth_discrim.hpp"

//#include <cv.h>
//#include <highgui.h>

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
            double sample_freq_res = (1.e6 * io->get_record_frequency());
            std::cout << "[normal] file time width is <" << sample_time_res << ">" << endl;
            std::cout << "[normal] file frequency width is <" << sample_freq_res << ">" << endl;

            // calculate number of frequency bins
            std::size_t freq_bins = env->get_freq_res();
            cout << "[normal] using <" << freq_bins << "> frequency bins for a resolution of <" << sample_freq_res / freq_bins << ">" << std::endl;

            // calculate number of time bins
            std::size_t time_bins = env->get_time_res();
            cout << "[normal] using <" << time_bins << "> time steps for a resolution of <" << sample_time_res / (sample_count / time_bins) << ">" << std::endl;

            // allocate channel signals
            std::cout << "[normal] allocating channel signals" << std::endl;
            glth::signal ch1( sample_count );
            glth::signal ch1_aa( sample_count );
            glth::signal ch2( sample_count );
            glth::signal ch2_aa( sample_count );
            glth::signal xwvd( freq_bins );

            // allocate transformer
            std::cout << "[normal] allocating transformer" << endl;
            glth::glth_xfrmr xfm( sample_count, freq_bins, time_bins );

            // open output
            std::ofstream output("./glth_out.dat");

            glth_const::io_result proc_res;
            std::size_t evt = 0;
            std::size_t offset;
	    proc_res = io->populate( ch1, ch2 );
	    //            {
                std::cout << "[normal] processing event <" << evt << ">..." << std::endl;

                // Calculate the analytic associate signal for each channel
                std::cout << "[normal] calculating analytic associate..." << std::endl;
                xfm.aa( ch1, ch1_aa );
                xfm.aa( ch2, ch2_aa );

                // Calculate the cross wigner ville distribution for the current event
                std::cout << "[normal] calculating wigner-ville distributions..." << std::endl;
                offset = 0;
                while( offset < sample_count - freq_bins )
                {
                    xfm.xwvd( ch1_aa, ch2_aa, xwvd, offset );
                    for( size_t i = 0; i < freq_bins / 2; i++ )
                    {
                        output << offset << " " << i << " " << xwvd[i][0]*xwvd[i][0]+xwvd[i][1]*xwvd[i][1] << "\n";
                    }
                    offset += time_bins;
                }
		//            }

            output.close();
        }
    }

    return 0;
}
