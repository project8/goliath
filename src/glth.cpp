#include <iostream>
#include <map>
#include <list>
#include "glth_io.hpp"
#include "glth_env.hpp"
#include "glth_signal.hpp"
#include "glth_tfr_data.hpp"
#include "glth_xfrmr.hpp"
#include "glth_candidate.hpp"
#include "glth_color.hpp"
#include "glth_discrim.hpp"
#include "glth_util.hpp"
#include "glth_tfr_map.hpp"
#include "glth_geom.hpp"
#include <cv.h>
#include <highgui.h>

using glth::env;
using glth::io;
using glth::tfr_data;
using glth::candidate;
using glth::discriminator;
using glth::tfr_map;
using glth::segment;
using namespace glth_const;

// For statistics and all
typedef std::pair< double, double > meanvar;

std::size_t next_po2( double arg );

int main( int argc, char** argv )
{
    // By default we have succeeded.
    exit_status res = exit_success;

    // OK now parse the arguments passed by the user and
    // verify that they are sensible.
    env const* env = env::env_from_args( argc, argv );

    if( env::verify( *env ) != env_success )
    {
        std::cout << "failed to create valid environment!" << std::endl;
        res = exit_env_failed;
    }

    else
    {
        // Our pointer to the IO object which gets our data for us
        io* io = io::open_file( env->get_in_filename() );

        // Signals!
        std::size_t record_len = (io->get_record_length());
        std::cout << "*Record length: " << record_len << std::endl;
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
            double nyquist_f = (io->get_record_frequency())/2.0;
            std::cout << "*Nyquist frequency is " << nyquist_f << "MHz" << std::endl;

	    std::size_t nbins = env->get_freq_res();
            if( nbins > 4096 )
            {
	      std::cout << "**>nbins too large! (" << nbins << ")" << ", adjusting to 4096" << std::endl;
	      nbins = 4096;
            }
            std::cout << "**Using " << nbins << " FFT frequency bins" << std::endl;
            float freq_res = nyquist_f * 1.0e6 / (double) nbins;
            std::cout << "**Actual frequency resolution is " << floor( freq_res ) << "Hz" << std::endl;

            // We need a transformer
            glth::glth_xfrmr xfm( record_len, nbins, env->get_time_res() );

            std::cout << "**Allocating " << record_len << "x" << nbins << " TFR data block" << std::endl;

	    // The TFR map is the workhorse data structure for searching through the XWVD.
	    // It maps an integer (a position in time) to a signal which is the FFT of the
	    // XWVD calculated at that time.
            tfr_map tfr_m;

	    // We also use a TFR map for the background analysis which takes place at the
	    // beginning of each record.
	    tfr_map tfr_bkg_m;

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
                std::cout << "**Calculating XWVD for background analysis..." << std::endl;
		glth::signal bkg_slice(nbins);
		for(std::size_t time = 0; time < 1024; time++) {
		  xfm.xwvd(ch1,ch2,bkg_slice,time);
		  tfr_bkg_m.insert(std::make_pair(time,bkg_slice));
		}

                // Calculate the adaptive threshold that's appropriate for this
                // event.  In the future this should be more robust, but for now,
                // we use the first 1024 elements of the TFR.  Basically we assume that
                // the noise in between 10 and 90 MHz is white, and so the power in
                // those bins is distributed normally.  This allows us to calculate
                // n-sigma thresholds to use as discriminators for each bin in that range.
                // We use a map (int -> double) for our threshold, where int is a bin index
                // and double is the threshold to use.  By using bin-by-bin thresholds,
                // we can escape the influence of the (unknown) receiver transfer function
                // in the frequency ROI.

                // First we calculate which bin corresponds to 10 and 100 MHz.
                std::size_t bin10MHz, bin100MHz;

                // The bin spacing is Nyquist/resolution, given by nbins above.
                // So resolution*index = frequency for that bin.
                bin10MHz = 10.0e6 / freq_res;
                bin100MHz = 100.0e6 / freq_res;

                // In addition, we care about the maximum power in the event.
                double max( 0.0 );

                std::cout << "**Calculating event threshold between bins " 
			  << bin10MHz 
			  << " and " 
			  << bin100MHz;
                std::map< int, meanvar > thresh;
                // Go between the bins we care about and calculate the thresholds for each
                for( std::size_t bin = bin10MHz; bin < bin100MHz; bin++ )
                {
                    for( std::size_t thresh_t = 0; thresh_t < 1024; thresh_t++ )
                    {
		      thresh[bin].first += glth::cplx_norm( (tfr_bkg_m.find(thresh_t)->second)[bin] );
                    }
                    // Calculate the mean
                    thresh[bin].first /= 1024;

                    // One for pass for variance
                    for( std::size_t thresh_t = 0; thresh_t < 1024; thresh_t++ )
                    {
		      double contrib = pow( glth::cplx_norm( (tfr_bkg_m.find(thresh_t)->second)[bin] ), 2 );
                        thresh[bin].second += contrib;
                    }
                    // Calculate the variance
                    thresh[bin].second /= 1024;
                    thresh[bin].second -= pow( thresh[bin].first, 2.0 );
                    thresh[bin].second = sqrt( thresh[bin].second );
                } // calculate thresholds bin by bin

                // Output mean of the means, mean of the variances as diagnostic.
                double meanmean( 0 ), meanstd( 0 );
                for( std::size_t bin = bin10MHz; bin < bin100MHz; bin++ )
                {
                    meanmean += thresh[bin].first;
                    meanstd += thresh[bin].second;
                } // calculate mean of mean and mean of standard deviation

                std::cout << "(" << "avg mean: " 
			  << meanmean 
			  << ", avg 1 sigma thresh:" 
			  << meanstd << ")" << std::endl;

                // Now traverse the XWVD from bin 1024 on.  At each time slice, inspect
                // each bin in the bandpass of the receiver and check if it is at the
                // 3 sigma level or higher (this is an arbitrary choice for now, this
                // should really be an option).  If it is, set the bit in the frequency
                // discriminator corresponding to that bin to high.  At that point, jump
		// forward by a chunk of time (wvd_coarse_stride), recalculate the XWVD
		// at that time, and then examine the bins defined by the chirp cone.
		// If any of those bins are high, jump backward by half the stride from
		// the new position (so that you now at time + stride/2) and look for
		// points which "connect" the two planes in the sense that they fall on
		// a line which connects high bins.  If those exist, jump forward by 
		// stride/4 (this is a binary search) and repeat.  If connections are
		// found between those two planes, declare that we have found an event.
		// At a 3 sigma threshold, this is randomly triggered at a rate of 1 in
		// 10^10.  Return to the furthest time position and repeat.

		// This is the coarse stride.  
		int wvd_coarse_stride = 2500;

                // We need to define a chirp cone.  For a linear FM chirp with a 
		// linear modulation b, such that f(t) = a + bt, if bin (a) is high
		// at time 0, then at time t, bin a+bt is high (obviously).  If we accept
		// b as a *parameter* that we are searching over, then we can accept a 
		// bmin and bmax, which defines the linear FM range that we can detect.
		// For example, if we say that the frequency changes no less than 
		// a part per thousand per time slice and no more than 100 parts per thousand,
		// then 2500 bins away, we look between a+2 and a+25000.  This is obviously 
		// dramatic but illustrates the point.  fm_min and fm_max are below in 
		// frequency bins / time bin, and can be derived at run time although they
		// are not here.
		double fm_min(0.001), fm_max(0.003);
		
		// These are the upper and lower edges of the chirp cone at a given plane.
		std::size_t upper_c, lower_c;

                // The discriminator banks.  We need 4 of them max.
		std::vector<discriminator> planes;
		for(int i = 0; i < 4; i++) {
		  planes.push_back(discriminator(nbins));
		}

                // This is our list of candidates.
                std::vector< candidate > cs;

		// Start after the background calculated stuff.
                int t0 = 1024;
		int t = t0;

		// The current stride.  Right now it's the coarse stride.
		int stride = wvd_coarse_stride;

		// Temporary calculation signal
		glth::signal loc_xwvd(nbins);

		// The recursion depth is specified by this index.
		int search_idx = 0;

		// Bools for finding an event.
		bool evt_found = false;
		bool stop_search = false;

		// Segments that we have found so far. 
		std::list<segment> sgs;

		// Discriminators based on search index.  If the search at index
		// i succeeds, search_pass[i] == true.
		discriminator search_pass(4);

		// GO GO GO 
		std::cout << "**Entering segmented search..." << std::endl;

		while( t < record_len ) {
		  std::cout << "t = " << t << std::endl;

		  // Calculate the XWVD of the two channels at this time slice.
		  xfm.xwvd(ch1,ch2,loc_xwvd,t);

		  // Sic the discriminator on it.  Go bin by bin in the frequency domain
		  // and check if the XWVD meets the threshold requirement.
		  for(std::size_t f = 0; f < nbins; f++) {

		    // Are we 3 sigma above?
		    if( glth::cplx_norm(loc_xwvd[f]) >= 
			(3.0*thresh[f].second + thresh[f].first) 
			|| glth::cplx_norm(loc_xwvd[f]) <= 
			(thresh[f].first - 3.0*thresh[f].second) ) {
		      planes[search_idx][f] = true;
		      if(search_idx == 0) search_pass[0] = true;
		    }
		  } // For loop over frequency bins

		  // If the search index is greater than 0, then we are binary searching.
		  // At plane 1, consider the points above threshold that are within the
		  // chirp cone of points above threshold in plane 0.  For each one, add
		  // a segment to the list of possible events.  
		  if( search_idx == 1 ) {
		    std::cout << "***Searching at depth 1..." << std::endl;

		    // Loop over bins in plane 0 and compare them to bins in plane 1
		    // that are high.
		    for( int f0 = 0; f0 < nbins; f0++ ) {
		      upper_c = (int)(f0 + fm_max*stride);
		      lower_c = (int)(f0 + fm_min*stride);
		      // if a bin in plane 0 is high, look inside the chirp cone of
		      // plane 1.
		      if(planes[0][f0] == true ) {
			for(std::size_t f1 = lower_c; f1 < upper_c; f1++) {
			  if((f1 < nbins) && (planes[1][f1] == true)) {
			    sgs.push_back(glth::segment(0,f0,wvd_coarse_stride,f1));
			  }
			} // loop over chirp cone bins
		      } // if plane 0 is high
		    }
		    // If there are segments to consider, enter the next recursion stage.
		    if( sgs.size() > 0 ) {
		      std::cout << "****" << sgs.size() 
				<< " segments found at search index 1, proceeding..."
				<< std::endl;
		      search_pass[1] = true;
		    }
		    else {
		      stop_search = true;
		    }
		  } // search index is 1

		  // If the search index is two, then we want to start investigating the
		  // segments found in the previous search.  For each segment, if a bin
		  // in the discriminator for plane 2 is high and that bin is located near
		  // where the segment intersects plane 2, keep the segment.  Otherwise,
		  // drop it.
		  if( search_idx == 2 ) {
		    std::list<segment>::iterator sg_it = sgs.begin();
		    while(sg_it != sgs.end()) {
		      // Ask the segment at what point it intercepts the current time plane.
		      int icept = (*sg_it).value_at((float)wvd_coarse_stride/2);

		      // std::cout << sg_it->x1()
		      // 		<< ","
		      // 		<< sg_it->y1()
		      // 		<< " : "
		      // 		<< sg_it->x2()
		      // 		<< ","
		      // 		<< sg_it->y2()
		      // 		<< " : "
		      // 		<< sg_it->slope()
		      // 		<< "," 
		      // 		<< icept
		      // 		<< std::endl;

		      // If the point on the intercept is not true, then erase the element
		      if(planes[2][icept] != true ) sg_it = sgs.erase(sg_it);
		      else sg_it++;
		    }
		    
		    // Now report the number of elements and either go to level 3 if we still
		    // have some candidates or stop searching.
		    if(sgs.size() > 0) {
		      std::cout << "***Search depth 2 finished with " 
				<< sgs.size() << " elements.  Proceeding..."
				<< std::endl;
		      search_pass[2] = true;
		    }
		    // Flag that we should stop.
		    else {
		      stop_search = true;
		    }
		  } // Search index 2 finishes
		  
		  // Search level 3.  One more check here.  We again ensure that the
		  // planes are connected at this time slice.
		  if( search_idx == 3 ) {
		    std::list<segment>::iterator sg_it = sgs.begin();
		    while(sg_it != sgs.end()) {
		      // Ask the segment at what point it intercepts the current time plane.
		      int icept = (*sg_it).value_at((float)wvd_coarse_stride*3.0/4);

		      // If the point on the intercept is not true, then erase the element
		      if(planes[3][icept] != true ) sg_it = sgs.erase(sg_it);
		      else sg_it++;
		    }
		    
		    // Now report the number of elements and either go to level 3 if we still
		    // have some candidates or stop searching.
		    if(sgs.size() > 0) {
		      std::cout << "***Search depth 3 finished with " 
				<< sgs.size() << " elements.  Proceeding..."
				<< std::endl;
		      search_pass[3] = true;
		    }
		    // Flag that we should stop.
		    else {
		      stop_search = true;
		    }
		  }

		  // Recalculate the stride.
		  std::cout << "search_idx = " << search_idx << std::endl;
		  stride = wvd_coarse_stride/(search_idx + 1);
		  stride = ((search_idx % 2 == 0) ? stride : -stride);
		  if( search_pass[search_idx] == true ) search_idx++;

		  // If we should stop because one of the levels exited or because we have
		  // found an event, exit.
		  if(stop_search == true || search_idx > 3 ) {
		    if( search_idx > 3) evt_found = true;
		    stop_search = false;
		    std::cout << "**Segment search stopped at level " 
			      << search_idx
			      << "... advancing."
			      << std::endl;
		    search_idx = 0;
		    stride = wvd_coarse_stride;
		    search_pass[0] = true;
		    sgs.clear();
		    for(int i = 1; i < 4; i++) search_pass[i] = false;
		  }
		  
		  // Go to the next time slice.
		  std::cout << "stride = " << stride << std::endl;
		  t += stride;

		} // While t is less than the record length

                // Increment the event counter.
                evt++;

            } // while populate

        } // if(io)

        // Otherwise opening the file barfed.  Exit ungracefully.
        else
        {
            std::cout << "couldn't open file: " << env->get_in_filename() << std::endl;
            res = exit_io_failed;
        }

    }
    return res;
}

std::size_t next_po2( double arg )
{
    int res = pow( 2, ceil( log2( arg ) ) );
    return (std::size_t) res;
}
