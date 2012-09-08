#include "glth_segment.hpp"

glth::segment glth::make_segment(std::size_t x1,
				 std::size_t x2,
				 std::size_t y1)
{
  return std::make_pair(std::make_pair(x1,x2),y1);
}
