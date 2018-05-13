#define PGMTEXTFEA_H
#ifdef PGMTEXTFEA_H

#include "data_sets/libpgm.h"

typedef struct datapgm {
  gray **image;
  int COLS;
  int ROWS;
  gray MAXVAL;
} pgm_image;

typedef struct datauser {
  int frec;
  int cells;
  float of_frec;
  float of_cells;
  char filter;
  int grey;
  int hder;
  int vder;
  int slope;
}input_user;


float **Process_Image(input_user *iuser, pgm_image *idata);

#endif
