#include "glth_discrim.hpp"

glth::discriminator::discriminator(std::size_t size) 
  : _bank(size)
{ 
  for(std::size_t i = 0; i < size; i++) _bank[i] = false;
}

bool& glth::discriminator::operator[](std::size_t index)
{
  return (this->_bank)[index];
}

bool glth::discriminator::any(std::size_t low, std::size_t high)
{
  for(std::size_t i = low; i < high; i++) {
    if ((this->_bank)[i] == true) return true;
  }
  return false;
}
