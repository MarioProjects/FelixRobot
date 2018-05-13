typedef struct{
  FILE *ifd,*ofd;
  int version;
  int verbosity;
  char method;
  float threshold;
  int rows, cols;
} pgmslant_values;

unsigned char **pgmslant(pgmslant_values, unsigned char **input, int *out_rows,
    int *out_cols);  
