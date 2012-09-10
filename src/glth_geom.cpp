#include "glth_geom.hpp"

glth::point::point() :
  _x(0), _y(0) 
{ /* just to satisfy the compiler, cannot be called */ }

glth::point::point(int x1, int y1) :
  _x(x1), _y(y1)
{ /* no-op */ }

const int glth::point::x() 
{
  return (this->_x);
}

const int glth::point::y() 
{
  return (this->_y);
}

glth::segment::segment() :
  _p1(point(0,0)), _p2(point(0,0)), _slope(0.0)
{
  /* no-op, just to satisfy the compiler, cannot be called */
}

glth::segment::segment(int x1, int y1, int x2, int y2) :
  _p1(point(x1,y1)), 
  _p2(point(x2,y2)), 
  _slope((float)(y2 - y1)/((float)(x2-y1)))
{ /* no-op */ }

const float glth::segment::slope() 
{
  return(this->_slope);
}

float glth::segment::value_at(float xval)
{
  return ((this->_p1).y() + (this->_slope)*xval);
}

const int glth::segment::x1()
{
  return ((this->_p1).x());
}

const int glth::segment::y1()
{
  return ((this->_p1).y());
}

const int glth::segment::x2()
{
  return ((this->_p2).x());
}

const int glth::segment::y2()
{
  return ((this->_p2).y());
}
