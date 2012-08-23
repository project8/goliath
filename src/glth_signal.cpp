#include "glth_signal.hpp"

// This is just for the compiler - we want signals to be 
// not constructible without either an analog or digital parameter
template <glth_types::glth_signal_type T>
glth::signal<T>::signal() 
{}

// We have to define two different constructors here because 
// each is now a concrete type.  
glth::signal<glth_types::digital>::signal(std::size_t len) 
  : _sigdata(len), _siglen(len)
{}

glth::signal<glth_types::analog>::signal(std::size_t len) 
  : _sigdata(len), _siglen(len)
{}

glth_types::byte& glth::digital_signal::operator[](std::size_t pos) {
  return ((*this)._sigdata)[pos];
}

double& glth::analog_signal::operator[](std::size_t pos) {
  return ((*this)._sigdata)[pos];
}

std::size_t glth::analog_signal::len() {
  return (*this)._sigdata.size();
}

std::size_t glth::digital_signal::len() {
  return (*this)._sigdata.size();
}

double* glth::analog_signal::data() 
{
  return (&(*this)._sigdata.front());
}

glth_types::byte* glth::digital_signal::data() 
{
  return (&(*this)._sigdata.front());
}

glth::digital_signal* analog_to_digital(glth::analog_signal in) {
  std::size_t newlen = in.len();
  glth::digital_signal *digout = new glth::digital_signal(newlen);
  if(digout) {
    for(std::size_t idx = 0; idx < newlen; idx++) {
      glth_types::byte pt = 0;
      if( in[idx] > glth_px1500::max_mvolts ) {
	pt = 255;
      }
      else if( in[idx] < glth_px1500::min_mvolts ) {
	pt = 0;
      }
      else {
	pt = 128 + in[idx]*(2<<glth_px1500::n_bits)/(glth_px1500::in_range);
      }
      (*digout)[idx] = pt;
    }
  }

  return digout;
}

glth::analog_signal* digital_to_analog(glth::digital_signal in) {
  std::size_t newlen = in.len();
  glth::analog_signal *anaout = new glth::analog_signal(newlen);
  if(anaout) {
    for(std::size_t idx = 0; idx < newlen; idx++) {
      double pt = 0;
      pt = glth_px1500::min_mvolts + 
	glth_px1500::max_mvolts*in[idx]/(2<<glth_px1500::n_bits);
    }
  }

  return anaout;
}
