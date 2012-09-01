#include "glth_env.hpp"

glth::env::env() 
  : _conf_mask(0)
{ /* no-op */ }

glth::env const* glth::env::env_from_args(int argc, char** argv) {
  glth::env* res = new glth::env();

  int c;
  while( (c = getopt(argc, argv, "i:")) != -1 ) {
    switch (c) {
    case 'i':
      res->set_in_filename(optarg);
      (res->_conf_mask) |=  _name_set;
      break;
    }
  }

  return res;
}

void glth::env::set_in_filename(std::string new_filename) {
  this->_in_filename = new_filename;
}

std::string glth::env::get_in_filename() {
  return static_cast<const env*>(this)->get_in_filename();
}

std::string glth::env::get_in_filename() const {
  return (this->_in_filename);
}

glth_const::env_result glth::env::verify(const glth::env tgt) {
  glth_const::env_result res = glth_const::env_success;

  // Check that the filename is set.
  if( (tgt._conf_mask & _name_set) != _name_set) {
    res = glth_const::env_no_filename;
  }

  return res;
}
