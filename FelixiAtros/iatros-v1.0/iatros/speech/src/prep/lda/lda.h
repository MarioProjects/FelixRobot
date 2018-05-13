#ifndef  LDA_H
#define  LDA_H

#include <iatros/features.h>

/*************************  LO  LAILO,  LO  LA  !!!  *********************/
#define  tamW  (int)  2
#define  tamW2  (int)  4
#define  tamW3  (int)  6

/**  tamBuf  =  (3*tamW+1)  ***/
#define   BUF_DERIV (int)  10

/*
#define  N_CMP  (int)  11
#define  N_CMP2  (int)  22
#define  N_CMP3  (int)  33
*/

/***  SIGMA  =  2*  SUM(i=1;i  <=  tamW;  i++)  i²  **********/
#define  SIGMA  (int)  10

typedef struct {
  double **matrix;
  int   size;
  int   window_width;
} lda_t;

lda_t * lda_create();
void lda_delete(lda_t * lda);
int lda_load(lda_t *lda, const char *filename, int n_ceps);
void lda_transform(const lda_t *lda, features_t *features);

#endif
