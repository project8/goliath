#ifndef __glth_segment_hpp
#define __glth_segment_hpp

#include <utility>
#include <cstddef>

namespace glth {

  class point {
  private:
    point();
    const int _x,_y;
  public:
    point(int x, int y);
    const int x();
    const int y();
  };

  class segment {
  private:
    segment();
    point _p1, _p2;
    const float _slope;
  public:
    segment(int x1, int x2, int y1, int y2);
    segment(point p1, point p2);
    const float slope();
    const int x1();
    const int y1();
    const int x2();
    const int y2();
    float value_at(float xvalue);
  };

}; // namespace glth

#endif // __glth_segment_hpp
