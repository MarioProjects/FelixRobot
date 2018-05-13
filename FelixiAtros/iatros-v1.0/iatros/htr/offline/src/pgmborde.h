#ifndef PGMBORDE_H
#define PGMBORDE_H

typedef struct{
  FILE *ifd,*ofd;
  int verbosity;
  float l,r,a,d;
  int blanco;
  int rows, cols;
} pgmborde_values;

unsigned char **pgmborde(pgmborde_values *vals, unsigned char **input);

#endif
