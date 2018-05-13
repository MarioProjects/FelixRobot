typedef struct{
  FILE *ifd,*ofd;
  float p_asc;
  float p_des;
  int m_normalizacion;
  int verbosity;
  int out_ascii;
  int rows,cols;
} pgmnormsize_values;

unsigned char **pgmnormsize(pgmnormsize_values *vals, unsigned char **input);

