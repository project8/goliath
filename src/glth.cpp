#include <iostream>
#include <map>
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

using glth::env;
using glth::io;
using glth::tfr_data;
using glth::candidate;
using glth::discriminator;
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

	// In addition, we care about the maximum power in the event.
	double max(0.0);

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
	// should really be an option).  If it is, set the bit in the frequency
	// discriminator corresponding to that bin to high.  In addition, check
	// the nearest neighbors of the frequency in the discriminator values
	// for the previous time slice.  If they are high, then multiply an
	// accumulator(prob_acc) by the probability of random occurrence (rand_prob):
	// P(random) = P(bin is high) && P(at least one of previous bins is high)
	// P(random) = disc_thresh*(1 - P(none of previous bins are high))
	// P(random) = disc_thresh*(1 - (1-disc_thresh)^N)
	// In addition, we set a bit that indicates we think that an event may be
	// forming(ev_trig), and record the time that this happened(t_trig).
	// Should the value of the accumulator fall below a threshold,
	// we set a bit that indicates that an event is detected(ev_det), 
	// record the point where it does, and once the search stops finding
	// correlated bins, we record it as a candidate.

	// The random probability as above.
	int n_neighbors = 3;
	double rand_thresh = 0.003; // Probability of being three sigma
	double rand_prob = rand_thresh*(1 - pow((1-rand_thresh),2*n_neighbors + 1));

	// The discriminator banks.
	discriminator f_disc(nbins), f_disc_last(nbins), t_disc(record_len);

	// The trigger bools.
	bool ev_trig(false), ev_det(false);

	// Time that the event was triggered at.
	std::size_t trig_init_t = 0;
	std::size_t fired_at=0;

	// The probability accumulator and threshold.
	double prob_acc = 1.0;
	double prob_thresh = 0.01;

	// This is our list of candidates.
	std::vector<candidate> cs;

	std::size_t t0 = 1024;
	// Iterate over time slices
	for(std::size_t t = t0; t < record_len; t++) {
	  // Temporary variable for power
	  double norm_pwr(0.0);

	  // A temporary that indicates that a frequency discriminator fired
	  // at this time slice.
	  bool time_disc = false;

	  // Inside, iterate over frequency
	  for(std::size_t f = bin10MHz; f < bin100MHz; f++) {
	    // Precalculate norm of bin
	    norm_pwr = std::norm(tfr[t][f]);

	    // If any bin in the time slice is high, record the frequency as high
	    if(norm_pwr >= (thresh[f].first + 3.0*thresh[f].second)) {
	      time_disc = true;
	      f_disc[f] = true;
	    }

	    // Check the value against the maximum.
	    if(norm_pwr > max) max = norm_pwr;

	    // Check if neighboring bins in the previous time slice were high, and
	    // if they were, record the time that this happened (unless we already
	    // suspect an event is forming)
	    if( f_disc_last.any(f-n_neighbors,f+n_neighbors) ) {
	      // If ev_trig hasn't been fired, fire it.
	      if(ev_trig == false) {
		trig_init_t = t;
		ev_trig = true;
	      }
	      prob_acc *= (1.0 - rand_prob);
	    } // nearest neighbor search

	    // Otherwise the nearest neighbor search failed.
	    else {
	      ev_trig = false;
	      prob_acc = 1.0;
	    }
	  } // for loop over frequency

	  // If the probability accumulator has fallen below threshold, then we
	  // declare that we detected something!
	  if( (prob_acc < prob_thresh) && (ev_det == false) ) {
	    std::cout << "***Event #" << evt 
		      << " passes cut at t=" << t 
		      << "(p < "
		      << prob_thresh
		      << ")"
		      << std::endl;
	    ev_det = true;
	  }
	  
	  // If we are currently in detection mode (i.e. ev_det is high), but
	  // we didn't see anything interesting in this time slice (i.e. 
	  // ev_trig is low), then we declare the event is over.  
	  // store the information about it, reset all of the counters
	  // and triggers, and move on.
	  if( (ev_det == true) && (ev_trig == false) ) {
	    std::cout << "***Event #" << evt
		      << " falls below threshold at t=" << t
		      << " duration="
		      << fired_at - t
		      << std::endl;
	    
	    cs.push_back(glth::candidate(fired_at, t, evt));

	    // Drop the event detection bool.
	    ev_det = false;

	    // Reset the probability accumulator.
	    prob_acc = 1.0;
	  }

	  // Set the last frequency discriminator to the current one.
	  f_disc_last = f_disc;
	} // for loop over time

	// It may be the case that we wind up at the end of the event
	// and the discriminator never went below threshold!  In that case,
	// add whatever is in the discriminator as an event.
	if( (ev_det == true) ) {
	  std::cout << "***Event #" << evt
		    << " ended with high trigger. "
		    << "Adding to candidate list: "
		    << "duration: "
		    << (record_len -1) - fired_at
		    << std::endl;
	  cs.push_back(glth::candidate(fired_at, record_len - 1, evt));
	}

	/*
	 * Now we have a list of candidates for this event.  We want to iterate
	 * over them, find a sensible packing, and then write that data to file
	 * so that humans can look at it.  Our iterator should be aligned at the
	 * first candidate that belongs to the current event.
	 */
	std::size_t c_idx(0);
	std::vector<candidate>::iterator cs_it;
	for( cs_it = cs.begin(); cs_it != cs.end(); cs_it++ ) {
	  glth::span idcs = (*cs_it).get_time_span();
	  std::size_t range = idcs.second - idcs.first;
	  cv::Mat out(nbins, range, CV_8UC3, cv::Scalar(0,0,0));

	  // Iterate and write.
	  for(std::size_t time = 0; time < range; time++) {
	    for(std::size_t freq = 0; freq < nbins; freq++) {
	      out.at<cv::Vec3b>((nbins - 1) - freq,time) =
		glth::jet(std::norm(tfr[time + idcs.first][freq]),max);
	    }
	  }

	  // Now write it.
	  std::stringstream fname;
	  fname << "glth_out_" 
		<< env->get_in_filename()
		<< "_"
		<< c_idx << ".png";
	  cv::imwrite(fname.str(), out);
	  
	  c_idx++;
	}
       
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
