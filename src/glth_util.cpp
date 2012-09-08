#include "glth_util.hpp"

double glth::cplx_norm(const glth::cplx tgt)
{
  return sqrt(tgt[0]*tgt[0] + tgt[1]*tgt[1]);
}
