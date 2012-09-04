#ifndef __glth_color_hpp
#define __glth_color_hpp

#include "cv.h"

namespace glth {

  /*
   * The jet color map.  Blue -> Red.
   */
  cv::Vec3b jet(double val, double valmax);

}; // namespace glth

#endif //  __glth_color_hpp
