#include "glth_color.hpp"

cv::Vec3b glth::jet(double v, double valmax) {
  double scale = valmax/4.0;
  unsigned char mid = 144;
  unsigned char red, blue, green;
  
  if( v < scale ) {
    blue = mid+(char)((255-mid)*v/valmax);
    red = green = 0;
  }
  else if( v < 2*scale ) {
    red=0;
    green=(unsigned char)(255*(v-scale)/scale);
    blue=255;
  }
  else if( v < 3*scale ) {
    red=(unsigned char)(255*(v-2*scale)/scale);
    green=255;
    blue=255-red;
  }
  else if( v < valmax ) {
    red=255;
    green=(unsigned char)(255-255*(v-3*scale)/scale);
    blue=0;
  }
  else {
    red = blue = green = 0;
  }

  return(cv::Vec3b(blue,green,red));
}
