#ifndef PGMRESTA_H
#define PGMRESTA_H

#include "data_sets/pgmClass.h"
#include "pbmclean.h"

class restaClass: public pgmimage {
public:
  void resta(const pgmimage & rlsaimage);
};

typedef struct{
  pbmclean_values v;
  int mean;
} pbmresta_values;

#endif
