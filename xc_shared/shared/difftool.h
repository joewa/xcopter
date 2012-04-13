#ifndef _DIFFTOOL_H_
#define _DIFFTOOL_H_

 #ifdef __cplusplus
 extern "C" {
 #endif

#define RES_FAC 256

typedef struct
{
  long x_k, x_k1;
  signed char xd_k, xd_k1;
} Diff1_s;

void diff1_init(Diff1_s *s);
signed char diff1(Diff1_s *s, long x);
long int1(Diff1_s *s, signed char xd);

typedef struct
{
  long x_k, x_k1, x_k2;
  long xd_k, xd_k1;
  signed char xdd_k2;
} Diff2_s;

signed char diff2(Diff2_s *s, long x);

 #ifdef __cplusplus
 }
 #endif

#endif
