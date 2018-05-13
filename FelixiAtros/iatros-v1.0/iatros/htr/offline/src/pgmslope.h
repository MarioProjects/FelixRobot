typedef struct{
  FILE *ifd,*ofd;
  float threshold;
  int version;
  int verbosity;
  int out_ascii;
  int global;
  int rows;
  int cols;
} pgmslope_values;

unsigned char **pgmslope(pgmslope_values *vals, unsigned char **input);

