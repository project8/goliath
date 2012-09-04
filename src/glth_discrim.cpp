#include "glth_discrim.hpp"

glth::discriminator::discriminator(std::size_t size) 
  : _bank(size)
{ /* no-op */ }

bool glth::discriminator::operator[](std::size_t index)
{
  return (this->_bank.at(index));
}
