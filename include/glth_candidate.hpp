#ifndef __glth_candidate_hpp
#define __glth_candidate_hpp

/*
 * The candidate class represents a contiguous set of
 * "hot" pixels in an event.  It hides information about
 * the beginning and end of a candidate event, as well
 * as the event number and so on.
 *
 * Candidates can be created from an event number, an 
 * initial time, and a final time.  Candidates can also
 * be "joined", such that the result of joining two 
 * candidates is a new candidate whose time indices 
 * span both, and whose event index spans both.  Using
 * this mechanism arbitrarily long candidates can be
 * built.
 */

#include <utility>
#include <cstddef>

namespace glth{

  // A "span" is a pair of indices
  typedef std::pair<std::size_t,std::size_t> span;

  class candidate {
  private:
    // The initial and final event number that the candidate is in.
    glth::span _evt_span;

    // The initial and final time index of the candidate
    glth::span _idx_span;

  public:
    
    // Basic constructor.
    candidate(std::size_t ti, std::size_t tf, std::size_t ev);
    candidate(std::size_t ti, std::size_t tf, std::size_t evi, std::size_t evf);

    // Join two candidates into one.
    static glth::candidate join(glth::candidate, glth::candidate);

    // Getters for event span, time span.
    glth::span get_event_span();
    glth::span get_time_span();

  }; // class candidate

}; // namespace glth

#endif // __glth_candidate_hpp
