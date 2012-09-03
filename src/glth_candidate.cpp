#include "glth_candidate.hpp"

glth::candidate::candidate(std::size_t ti, std::size_t tf, std::size_t ev)
  : _evt_span(ev, ev), _idx_span(ti,tf)
{ /* no-op */ }

glth::candidate::candidate(std::size_t ti, std::size_t tf, std::size_t evi, std::size_t evf)
  : _evt_span(evi, evf), _idx_span(ti,tf)
{ /* no-op */ }

glth::candidate glth::candidate::join(glth::candidate a, glth::candidate b)
{
  if(a.get_event_span() == b.get_event_span()) {
    return glth::candidate(a.get_time_span().first, 
			   b.get_time_span().second,
			   a.get_event_span().first);
  }
  else {
    return glth::candidate(a.get_time_span().first, 
			   b.get_time_span().second,
			   a.get_event_span().first,
			   b.get_event_span().second);
  }
}

glth::span glth::candidate::get_event_span() 
{
  return (*this)._evt_span;
}

glth::span glth::candidate::get_time_span() 
{
  return (*this)._idx_span;
}
