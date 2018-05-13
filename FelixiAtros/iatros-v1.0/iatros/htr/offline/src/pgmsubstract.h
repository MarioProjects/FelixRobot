#ifndef PGMSUBSTRACT_H
#define PGMSUBSTRACT_H

#include "data_sets/pgmClass.h"
// -------Datos ------------------------------------
class pgmsubstract :public pgmimage{
  pgmimage *median(int kernel) ;
public:
  void substract(int k);
  void substract(pgmimage &subs);
};

#endif
