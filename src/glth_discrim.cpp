#include "glth_discrim.hpp"

glth::discriminator::discriminator(std::size_t size) 
  : _bank(size)
{ /* no-op */ }

bool glth::discriminator::operator[](std::size_t index)
{
  return (this->_bank.at(index));
}

bool glth::discriminator::any(std::size_t low, std::size_t high)
{
  for(std::size_t i = low; i < high; i++) {
    if ((*this)[i]) return true;
  }
  return false;
}