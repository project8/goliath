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
		  << bin100MHz;
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
	    double contrib = pow(std::norm(tfr[thresh_t][bin]),2);
	    thresh[bin].second += contrib;
	  }
	  // Calculate the variance
	  thresh[bin].second /= 1024;
	  thresh[bin].second -= pow(thresh[bin].first,2.0);
	  thresh[bin].second = sqrt(thresh[bin].second);
	}

	// Output mean of the means, mean of the variances as diagnostic.
	double meanmean(0), meanstd(0);
	for(std::size_t bin = bin10MHz; bin < bin100MHz; bin++) {
	  meanmean += thresh[bin].first;
	  meanstd += thresh[bin].second;
	}
	
	std::cout << "(" 
		  << "avg mean: "
		  << meanmean
		  << ", avg 1 sigma thresh:"
		  << meanstd
		  << ")" 
		  << std::endl;
	
	// Now traverse the XWVD from bin 1024 on.  At each time slice, inspect
	// each bin in the bandpass of the receiver and check if it is at the 
	// 3 sigma level or higher (this is an arbitrary choice for now, this
	// should really be an option).  If it is, set a flag that a discriminator
	// was high.  There is, of course, a 0.3% chance for a bin to randomly 
	// fluctuate to 3 sigma, and so a 0.3% chance for *any* of the N bins inside
	// the bandpass to be high.  So for *any* bin to be high for M time slices
	// in a row, we have P = (0.003)^M.  This can be used as another kind of 
	// detection threshold: once you've observed a P < P' event, fire the top 
	// level discriminator.  P' is (0.003)^(f_s*t) where f_s is the sampling
	// frequency and t is a time constant.  This is now a probability for 
	// any bin to be high for some time t given a sampling frequency f_s.
	double probacc = 1.0;
	double probdisc = 0.3;
	bool threesig = false;
	bool time_high = false;
	std::size_t highfor=0, fired_at=0;

	std::size_t t0 = 1024;
	// Iterate over time slices
	for(std::size_t t = t0; t < record_len; t++) {
	  bool time_disc = false;
	  // Inside, iterate over frequency
	  for(std::size_t f = bin10MHz; f < bin100MHz; f++) {
	    // If any bin in the time slice is high, fire the inner discriminator
	    if(std::norm(tfr[t][f]) >= 3.0*thresh[f].second) {
	      time_disc = true;
	    }
	    // If we fired, record the time, UNLESS threesig is high, in which case
	    // forget it.
	    if( time_disc == true && threesig == false ) {
	      fired_at = t;
	    }
	  } // for loop over frequency

	  // If both the inner discriminator and the outer discriminator are high,
	  // we multiply the probability by the three sigma increment.
	  if( (time_disc & threesig) == true ) {
	    probacc *= (1.0 - 0.003);
	    // If probacc has fallen below threshold, make some noise.  If we've 
	    // alredy made noise, don't make any more.
	    if(probacc < probdisc) {
	      if( time_high == false ) {
		std::cout << "***Event #" << evt 
			  << " passes cut at t=" << t 
			  << "(p = "
			  << probacc
			  << ")"
			  << std::endl;
		time_high = true;
		highfor = t;
	      } 
	    }
	  } // time_disc & threesig
	  
	  // If time_disc is true but threesig is false, set threesig to true. 
	  // Otherwise, if time_disc is false but threesig is true, set threesig
	  // to false.  Basically set threesig to the state of time_disc.
	  else {
	    threesig = time_disc;

	    // If the time disciminator is high, then threesig was low, and 
	    // we start counting.
	    if(time_disc == true) {
	      probacc = 1.0;
	    }

	    // If the time discriminator was high, report that we went low and
	    // set it to false.
	    if((time_disc == false) && (time_high == true)) {
	      std::cout << "***Event fails cut at t=" 
			<< t 
			<< "(high for: "
			<< t - fired_at
			<< ")"
			<< std::endl;
	      time_high = false;
	      probacc = 1.0;
	    }
	  } // sync threesig to time_disc

	} // for loop over time

       
	// Increment the event counter.
	evt++;

      } // while populate
      
    } // if(io)

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
