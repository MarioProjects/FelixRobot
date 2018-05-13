#ifndef PGMRLSA_H
#define PGMRLSA_H

#include "data_sets/pgmClass.h"

class rlsaClass: public pgmimage {
  int otsu();
  float * histo_norm();
  void mean_double(float * h,int k, float & mu1, float & mu2, float & muT);
public:
  void rlsa(const int longMediaLetra);
};

#endif
