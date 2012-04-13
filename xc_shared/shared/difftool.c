/*typedef struct
{
  long x_k, x_k1;
  signed char xd_k;
} Diff1_s;*/
#include "difftool.h"

void diff1_init(Diff1_s *s)
{
  s->x_k = 0; s->x_k1 = 0; s->xd_k1 = 0; s->xd_k = 0;
}

signed char diff1(Diff1_s *s, long x)
{
  s->x_k1 = s->x_k;
  s->x_k  = x;
  s->xd_k1 = s->x_k - s->x_k1;
  return s->xd_k1;
}

long int1(Diff1_s *s, signed char xd)
{
  s->xd_k = xd;
  s->x_k1 = s->x_k;
  s->x_k += s->xd_k;
  return s->x_k;
}

/*typedef struct
{
  long x_k, x_k1, x_k2;
  long xd_k, xd_k1;
  signed char xdd_k2;
} Diff2_s;*/
signed char diff2(Diff2_s *s, long x)
{
  s->x_k2 = s->x_k1;
  s->x_k1 = s->x_k;
  s->x_k = x;
  //s->xd_k1 = ...
  s->xdd_k2 = s->x_k - 2 * s->x_k1 + s->x_k2;
  return s->xdd_k2;
}
