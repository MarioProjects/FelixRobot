#ifndef PGMMEDIAN_H
#define PGMMEDIAN_H

typedef struct{
  FILE *in,*out;
  int fast;
  int extension_even;
  int kernel;
  int rows;
  int cols;
} pgmmedian_values;

unsigned char **pgmmedian(pgmmedian_values *vals, unsigned char **input);

#endif


