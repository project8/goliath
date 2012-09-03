#include <iostream>
#include <map>
#include "glth_io.hpp"
#include "glth_env.hpp"
#include "glth_signal.hpp"
#include "glth_tfr_data.hpp"
#include "glth_xfrmr.hpp"

using glth::env;
using glth::io;
using glth::tfr_data;
using namespace glth_const;

// For statistics and all
typedef std::pair<double, double> meanvar;

std::size_t next_po2(double arg);

int main(int argc, char** argv) {
  // By default we have succeeded.
  exit_status res = exit_success;

  // OK now parse the arguments passed by the user and
  // verify that they are sensible.
  env const* env = env::env_from_args(argc, argv);

  if( env::verify(*env) != env_success ) {
    std::cout << "failed to create valid environment!" << std::endl;
    res = exit_env_failed;
  }

  else {
    // Our pointer to the IO object which gets our data for us
    io* io = io::open_file(env->get_in_filename());

    // Signals!
    std::size_t record_len = (io->get_record_length());
    std::cout << "*Record length: " << record_len << std::endl;
    std::cout << "**Allocating channel signals" << std::endl;
    glth::signal ch1(record_len), ch2(record_len);

    // IO being a valid pointer means we're in business.
    if(io) {
      // An event counter
      std::size_t evt = 0;

      // Create a TFR data object to hold the result of our
      // WVD.  This object will get reused for analysis on each event.
      // But how do we know how many frequency bins we need?  We use the
      // Nyquist rate from the header and our desired frequency resolution,
      // which in this case is hard coded at 30kHz.  We find the next highest
      // power of two and that's our binning.
      double nyquist_f = (io->get_monarch_ptr()->GetHeader()->GetAcqRate())/2.0;
      std::cout << "*Nyquist frequency is " << nyquist_f << "MHz" << std::endl;

      double freq_res = 30.0e3;
      std::size_t nbins = next_po2(nyquist_f*1.0e6/freq_res);
      if( nbins > 2048 ) {
	std::cout << "**>nbins too large! (" << nbins << ")"
		  << ", adjusting to 128" << std::endl;
	nbins = 128;
      }
      std::cout << "**Using " << nbins << " FFT frequency bins" << std::endl;
      freq_res = nyquist_f*1.0e6/(double)nbins; 
      std::cout << "**Actual frequency resolution is " << floor(freq_res) 
		<< "Hz" << std::endl;

      // We need a transformer
      glth::glth_xfrmr xfm(record_len, record_len, nbins);
      
      std::cout << "**Allocating " << record_len << "x" 
		<< nbins << " TFR data block" << std::endl;
      tfr_data tfr(record_len, nbins);

      // At this point we're ready to rock.  We grab records, convert them to
      // analog signals, and then XWVD the two channels.
      glth_const::io_result proc_res;
      while( (proc_res = io->populate(&ch1, &ch2)) == glth_const::io_read_ok ) {
	std::cout << "*Processing event #" << evt << std::endl;

	// Calculate the cross wigner ville distribution for the current event
	std::cout << "**Calculating XWVD..." << std::endl;
	xfm.xwvd(ch1,ch2,&tfr);

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
	bin10MHz = 10.0e6/freq_res;
	bin100MHz = 100.0e6/freq_res;

	std::cout << "**Calculating event threshold between bins " 
		  << bin10MHz
		  << " and "
		  << bin100MHz
		  << std::endl;
	std::map<int,meanvar> thresh;
	// Go between the bins we care about and calculate the thresholds for each  
	for(std::size_t bin = bin10MHz; bin < bin100MHz; bin++) {
	  for(std::size_t thresh_t = 0; thresh_t < 1024; thresh_t++) {
	    thresh[bin].first += std::norm(tfr[thresh_t][bin]);
	  }
	  // Calculate the mean
	  thresh[bin].first /= 1024;
	  
	  // One for pass for variance
	  for(std::size_t thresh_t = 0; thresh_t < 1024; thresh_t++) {
	    thresh[bin].second += pow(std::norm(tfr[thresh_t][bin]) - thresh[bin].first,2);
	  }
	  // Calculate the variance
	  thresh[bin].second /= 1024;

	  std::cout << thresh[bin].first << "," << thresh[bin].second << std::endl;
	  
	}
       
	// Increment the event counter.
	evt++;
      } // if(io)
      
    }

    // Otherwise opening the file barfed.  Exit ungracefully.
    else {
      std::cout << "couldn't open file: " 
		<< env->get_in_filename() 
		<< std::endl;
      res = exit_io_failed;
    }
   
  }
  return res;
}

std::size_t next_po2(double arg) 
{
  int res = pow(2,ceil(log2(arg)));
  return (std::size_t)res;
}
