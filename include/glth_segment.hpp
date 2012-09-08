#ifndef __glth_segment_hpp
#define __glth_segment_hpp

#include <utility>
#include <cstddef>

namespace glth {

  typedef std::pair<std::pair<std::size_t, std::size_t>, std::size_t> segment;

  segment make_segment(std::size_t x1,
		       std::size_t x2,
		       std::size_t y1);

}; // namespace glth

#endif // __glth_segment_hpp
