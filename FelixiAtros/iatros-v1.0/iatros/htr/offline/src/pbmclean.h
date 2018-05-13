#ifndef PBMCLEAN_H
#define PBMCLEAN_H

#include "data_sets/mfSetClass.h"
extern "C"{
#include "data_sets/libpbm.h"
}
typedef struct{
  FILE *ifd,*ofd;
  int out_ascii;
  int verbosity;
  int cols,rows;
  float pTall;
} pbmclean_values;

unsigned char **pbmclean(pbmclean_values *vals,unsigned char **input,bit black = PBM_BLACK, bit white = PBM_WHITE);

#endif
