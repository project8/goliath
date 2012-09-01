#include "glth_io.hpp"

glth::io::io() : _monarch_ptr(NULL)
{/* no-op */}

glth::io* glth::io::open_file(std::string filename) {
  glth::io* _io_ptr = new glth::io();

  _io_ptr->_monarch_ptr = Monarch::OpenForReading(filename);

  // If the monarch pointer is not NULL, we're good.
  if( _io_ptr->_monarch_ptr == NULL ) {
    delete _io_ptr;
    _io_ptr = NULL;
  }
  
  return _io_ptr;
}
