#ifndef  DERIVADES_H
#define  DERIVADES_H

#include <iatros/features.h>

#define  tamW  (int)  2
#define  tamW2  (int)  4
#define  tamW3  (int)  6

/**  tamBuf  =  (3*tamW+1)  ***/
#define  BUF_DERIV (int)  10
#define  DERIV  3 //Used in aachen derivative

/*
#define  N_CMP  (int)  11
#define  N_CMP2  (int)  22
#define  N_CMP3  (int)  33
*/

/***  SIGMA  =  2*  SUM(i=1;i  <=  tamW;  i++)  i²  **********/
#define  SIGMA  (int)  10

typedef enum {NO_DERIV = -1, HTK = 0, AACHEN, MAX_DERIV_TYPE} deriv_t;

void compute_derivatives(features_t *features, deriv_t type);
int DerivativeFrame(float *CC, float **Vec_CC, float **buffCC, int frame, int iCCs, deriv_t type);
int DerivativeFrameHTK(float *CC, float **Vec_CC, float **buffCC, int frame, int iCCs);
int DerivativeFrameAachen(float *CC, float **Vec_CC, float **buffCC,  int frame, int iCCs);

#endif
