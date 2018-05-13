typedef struct{
  FILE *ifd,*ofd;
  int verbosity;
  int rows,cols;
} pgmskew_values;

unsigned char **pgmskew(pgmskew_values *vals, unsigned char **input);

