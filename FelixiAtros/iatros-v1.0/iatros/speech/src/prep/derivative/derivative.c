/** @defgroup Derivative Derivative Module
 *  @ingroup prep
 *  Derivative fuction
 *  @{
 */

/** @page authors Authors
 *
 *       Authors:
 *              Moises Pastor (original version in ATROS)\n
 *              Vicent Tamarit (adaptation of the source to iATROS)\n
 */

/** @page explanation How is it calculated?
*\verbatim
*                 tamW
*                SUM ( i * (CC  -  CC    )
*                 i=i         t+1    t-i
*          dt = ---------------------------       if tamW < t <= T - tamW   (* que estiga entre les
*                         tamW                                               tamW primeres i les
*                    2*  SUM (i²)                                            tamW ultimes trames *)
*                         i=1
*
*
*
*          dt = CC   -  CC      if   t < tamW      (* que siguen les tamW primeres trames *)
*                 t+1     t
*
*
*
*          dt = CC  -  CC        if  t>= T - tamW  (* que siguen les tamW ultimeSe trames *)
*       Names:
*              tamW ->  window's length for the derivative
*              T    ->  size of the data
*              dt   ->  derivative number t
*              CCt  ->  cepstral number t
*\endverbatim
*/


#include "derivative.h"
#include <prhlt/trace.h>
#include <prhlt/constants.h>
#include <stdio.h>
#include <string.h>


void compute_derivatives(features_t *features, deriv_t type) {
  float **Vec_CC = (float **) malloc(sizeof(float *) * BUF_DERIV);
  for (int i = 0; i < BUF_DERIV; i++) {
    Vec_CC[i] = (float *) malloc(sizeof(float) * ((features->n_features) * DERIV));
  }

  float **buffCC = (float **) malloc(sizeof(float *) * BUF_DERIV);
  for (int i = 0; i < BUF_DERIV; i++) {
    buffCC[i] = (float *) malloc(sizeof(float) * ((features->n_features) * DERIV));
  }

  int i, numFrames, new_n_vectors = 0;
  for (i = 0; i < features->n_vectors; i++) {
    numFrames = DerivativeFrame(features->vector[i], Vec_CC, buffCC, i, features->n_features, type);

    if ((numFrames >= 1) && (numFrames < BUF_DERIV)) {
      for (int k = 0; k < numFrames; k++) {
        features->vector[new_n_vectors] = (float *)realloc(features->vector[new_n_vectors], features->n_features * DERIV * sizeof(float));
        memcpy(features->vector[new_n_vectors], Vec_CC[k], features->n_features * DERIV * sizeof(float));
        new_n_vectors++;
      }
    }
  }

  numFrames = DerivativeFrame(NULL, Vec_CC, buffCC, i - 1, features->n_features, type);
  if ((numFrames >= 1) && (numFrames < BUF_DERIV)) {
    if (new_n_vectors + numFrames > features->n_vectors) {
      features->vector = (float **)realloc(features->vector, (new_n_vectors + numFrames)*sizeof(float **));
      for (int j = features->n_vectors; j < new_n_vectors + numFrames; j++) {
        features->vector[i] = NULL;
      }
    }
    for (int k = 0; k < numFrames; k++) {
      features->vector[new_n_vectors] = (float *)realloc(features->vector[new_n_vectors], features->n_features * DERIV * sizeof(float));
      memcpy(features->vector[new_n_vectors], Vec_CC[k], features->n_features * DERIV * sizeof(float));
      new_n_vectors++;
    }
  }

  for (i = 0; i < BUF_DERIV; i++) {
    free(Vec_CC[i]);
    free(buffCC[i]);
  }
  free(Vec_CC);
  free(buffCC);

  // fill structure field
  char *structure = (char *)malloc(10*MAX_LINE*sizeof(char));
  strcpy(structure, "");

  if (features->structure) {
    strcat(structure, features->structure);
  }
  else {
    for (i = 0; i < features->n_features; i++) {
      sprintf(structure, "%s f%d", structure, i);
    }
  }
  for (i = 0; i < features->n_features; i++) {
    sprintf(structure, "%s der%d", structure, i);
  }
  for (i = 0; i < features->n_features; i++) {
    sprintf(structure, "%s acc%d", structure, i);
  }
  features->structure = (char *)realloc(features->structure, (strlen(structure)+1)*sizeof(char));
  strcpy(features->structure, structure);
  free(structure);

  // update feature parameters
  features->n_features = features->n_features * DERIV;
  features->n_vectors = new_n_vectors;
  features->type = FF_CC_DER_ACC;

}

int DerivativeFrame(float *CC, float **Vec_CC, float **buffCC, int frame, int iCCs, deriv_t type) {
  switch (type) {
    case HTK:
      return DerivativeFrameHTK(CC, Vec_CC, buffCC, frame, iCCs);
      break;

    case AACHEN:
      return DerivativeFrameAachen(CC, Vec_CC, buffCC, frame, iCCs);
      break;

    default:
      FAIL("Invalid derivative type\n");
      return 0;

  }
}

/// Computes the derivatives of a cepstrum vectors using the Aachen derivative
/**
   @param[in] CC Pointer to the Cepstrum vector
   @param[out] Vec_CC Buffer with the last vectors computed
   @param[in] buffCC Buffer of CC. This buffer is only used by this function
   @param[in] frame The number of the frame we are processing. In the last case (CC=NULL) the frame must be the number of last frame seen.
   @param[in] iCCs Number of Cepstra (lentgh of the CC vector)

   @return number of completed cepstrum vector
*/
int DerivativeFrameAachen(float *CC, float **Vec_CC, float **buffCC,  int frame, int iCCs)
 /* David  Mayo  96  a  partir  de  calc_diff  del  sw  de  Aachen  */
 /* Todavia  muy  optimizable  (memcopy...)  &&  FALTA  !  USAR  GLOBALES  !!
  *
  */
{
  int n_cmp3=3*iCCs;

  int cmp, tim;
  float Vec_CC_Aux[BUF_DERIV][n_cmp3];


  const int N_cmp_file = iCCs;
  const int N_cmp_first = iCCs;
  const int N_cmp_second = iCCs;

  if (CC != NULL) {
    /* copy  Vec_CC_Aux  to  bufferCC  */
    memcpy (buffCC[frame % BUF_DERIV], CC, sizeof (float) * N_cmp_file);

    if ((frame != -1) && (frame < 2 * DERIV)) return (0);
  }
  memcpy (Vec_CC_Aux[DERIV], buffCC[(frame - DERIV) % BUF_DERIV], sizeof (float) * N_cmp_file);

  /* first  derivatives  */
  for (cmp = 0; cmp < N_cmp_first; cmp++) {
    /* N_cmp_first  =  number  of  first  derivatives  in  feature  vector  */
    Vec_CC_Aux[DERIV][cmp + N_cmp_file] = buffCC[(frame - DERIV) % BUF_DERIV][cmp] -
      buffCC[(frame - 2 * DERIV) % BUF_DERIV][cmp];
  }

  /* second  derivatives  */
  for (cmp = 0; cmp < N_cmp_second; cmp++) {
    /* N_cmp_second  =  number  of  second  derivatives  in  feature  vector
     * * * */
    Vec_CC_Aux[DERIV][cmp + N_cmp_file + N_cmp_first] = buffCC[frame % BUF_DERIV][cmp] -
      2 * buffCC[(frame - DERIV) % BUF_DERIV][cmp] +
      buffCC[(frame - 2 * DERIV) % BUF_DERIV][cmp];
  }

  /*--------------------------------------------*/
  /* complete  buffCC  at  sentence  boundaries */
  /*--------------------------------------------*/

  if (frame == 2 * DERIV) {        /* principio  de  frase  */
    for (tim = 0; tim < DERIV; tim++) {
      /* copia  cepstales  */
      memcpy (Vec_CC_Aux[tim], buffCC[(tim) % BUF_DERIV], sizeof (float) * N_cmp_file);

      /* first  derivatives  */
      for (cmp = 0; cmp < N_cmp_first; cmp++)
        Vec_CC_Aux[tim][cmp + N_cmp_file] = Vec_CC_Aux[DERIV][cmp + N_cmp_file];

      /* second  derivatives  */
      for (cmp = 0; cmp < N_cmp_second; cmp++)
        Vec_CC_Aux[tim][cmp + N_cmp_file + N_cmp_first] = Vec_CC_Aux[DERIV][cmp + N_cmp_file + N_cmp_first];
    }
    for (int i=0; i < DERIV + 1; i++)
      memcpy (Vec_CC[i], Vec_CC_Aux[i], sizeof (float) * n_cmp3);

    return (DERIV + 1);
  }
  if (CC == NULL) {                /* marca  fin  de  frase  */
    /* copia  cepstales  */
    for (tim = DERIV + 1; tim <= 2 * DERIV; tim++)
      memcpy (Vec_CC_Aux[tim], buffCC[(frame - 2 * DERIV + tim) % BUF_DERIV], sizeof (float) * N_cmp_file);

    for (tim = DERIV + 1; tim <= 2 * DERIV; tim++) {
      /* first  derivatives  */
      for (cmp = 0; cmp < N_cmp_first; cmp++)
        Vec_CC_Aux[tim][cmp + N_cmp_file] = Vec_CC_Aux[tim][cmp] - buffCC[(frame - 3 * DERIV + tim) % BUF_DERIV][cmp];

      /* second  derivatives  */
      for (cmp = 0; cmp < N_cmp_second; cmp++)
        Vec_CC_Aux[tim][cmp + N_cmp_file + N_cmp_first] = Vec_CC_Aux[DERIV][cmp + N_cmp_file + N_cmp_first];
    }
    for (int i=0; i < DERIV + 1; i++)
      memcpy (Vec_CC[i], Vec_CC_Aux[DERIV + i], sizeof (float) * n_cmp3);

    return (DERIV + 1);
  }
  memcpy (Vec_CC[0], Vec_CC_Aux[DERIV], sizeof (float) * n_cmp3);

  return (1);
}


/// Computes the derivatives of a cepstrum vectors
/**
   @param[in] CC Pointer to the Cepstrum vector
   @param[out] Vec_CC Buffer with the last vectors computed
   @param[in] buffCC Buffer of CC. This buffer is only used by this function
   @param[in] frame The number of the frame we are processing. In the last case (CC=NULL) the frame must be the number of last frame seen.
   @param[in] iCCs Number of Cepstra (lentgh of the CC vector)

   @return number of completed cepstrum vector
*/
int DerivativeFrameHTK(float *CC, float **Vec_CC, float **buffCC,  int frame, int iCCs)
{
  int n_cmp=iCCs;
  int n_cmp2=2*iCCs;
  int n_cmp3=3*iCCs;

  //  static float buffCC[BUF_DERIV][3*iCCs];
  int i, j, k, despl;

  if (CC != NULL) {
    memcpy (buffCC[frame % BUF_DERIV], CC, sizeof (float) * n_cmp);

    if (frame < (tamW3 - 1)) {
      return 0;
    }
    /* anem  omplint  fins  que  tenim  prou  per  fer  algun  calcul  */
    else if (frame == tamW3 - 1) {

      /* Initial case */
      /* Calculate all that can be compute and left the circullar buffer ready
       for the general case. Return the first tamW frames */


      for (i = 0; i < tamW; i++)
        for (j = 0; j < n_cmp; j++)
          buffCC[i][n_cmp + j] = buffCC[i + 1][j] - buffCC[i][j];

      for (i = tamW; i < tamW2; i++) {
        for (j = n_cmp; j < n_cmp2; j++)
          buffCC[i][j] = 0;

        for (despl = 1; despl <= tamW; despl++)
          for (j = 0; j < n_cmp; j++)
            buffCC[i][n_cmp + j] += (buffCC[i + despl][j] - buffCC[i - despl][j]) * despl / SIGMA;
      }

      for (i = 0; i < tamW; i++)
        for (j = 0; j < n_cmp; j++)
          buffCC[i][n_cmp2 + j] = buffCC[i + 1][n_cmp + j] - buffCC[i][n_cmp + j];
      for(i=0;i<tamW;i++)
        memcpy (Vec_CC[i], buffCC[i], sizeof (float) * n_cmp3);

      return tamW;

    } else {

      /* General case */
      for (i = n_cmp; i < n_cmp2; i++)
        buffCC[(frame - tamW) % BUF_DERIV][i] = 0;

      for (despl = 1; despl <= tamW; despl++)
        for (j = 0; j < n_cmp; j++)
          buffCC[(frame - tamW) % BUF_DERIV][n_cmp + j] += (buffCC[(frame - tamW + despl) % BUF_DERIV][j] - buffCC[(frame - tamW - despl) % BUF_DERIV][j]) * despl / SIGMA;
      for (j = n_cmp2; j < n_cmp3; j++)
        buffCC[(frame - tamW2) % BUF_DERIV][j] = 0;

      for (despl = 1; despl <= tamW; despl++)
        for (j = 0; j < n_cmp; j++)
          buffCC[(frame - tamW2) % BUF_DERIV][n_cmp2 + j] += (buffCC[(frame - tamW2 + despl) % BUF_DERIV][n_cmp + j] - buffCC[(frame - tamW2 - despl) % BUF_DERIV][n_cmp + j]) * despl / SIGMA;

      memcpy (Vec_CC[0], buffCC[(frame - tamW2) % BUF_DERIV], sizeof (float) * n_cmp3);

      return 1;
    }
  } else {                        /* si  CC  ==  NULL  */

    /* Final case  */
    /* There is no new frame. Complete the buffer and returns the last frames
     */
    for (i = (frame - tamW + 1) % BUF_DERIV, k = 0; k < tamW; k++, i = (i + 1) % BUF_DERIV)
      for (j = 0; j < n_cmp; j++)
        buffCC[i][n_cmp + j] = buffCC[i][j] - buffCC[(i - 1 + BUF_DERIV) % BUF_DERIV][j];

    //This is old code, it's here for compare with the new one
    /*for (i = (frame - tamW2 + 1) % BUF_DERIV, k = 0; k > tamW; k++, i = (i + 1) % BUF_DERIV) {
      for (j = n_cmp2; j < n_cmp3; j++)
        buffCC[j][j] = 0;

      for (despl = 1; despl <= tamW; despl++)
        for (j = 0; j < n_cmp; j++)
          buffCC[i][n_cmp2 + j] += (buffCC[(i + despl) % BUF_DERIV][n_cmp + j] - buffCC[(i - despl) % BUF_DERIV][n_cmp + j]) * despl / SIGMA;
    }*/

    for (i = (frame - tamW2 + 1) % BUF_DERIV, k = 0; k < tamW; k++, i = (i + 1) % BUF_DERIV) {
      for (j = n_cmp2; j < n_cmp3; j++)
        buffCC[i][j] = 0;

      for (despl = 1; despl <= tamW; despl++)
        for (j = 0; j < n_cmp; j++)
          buffCC[i][n_cmp2 + j] += (buffCC[(i + despl) % BUF_DERIV][n_cmp + j] - buffCC[(i - despl + BUF_DERIV) % BUF_DERIV][n_cmp + j]) * despl / SIGMA;
    }

    for (i = (frame - tamW + 1) % BUF_DERIV, k = 0; k < tamW; k++, i = (i + 1) % BUF_DERIV)
      for (j = 0; j < n_cmp; j++)
        buffCC[i][n_cmp2 + j] = buffCC[i][n_cmp + j] - buffCC[(i - 1 + BUF_DERIV) % BUF_DERIV][n_cmp + j];

    for (i = (frame - tamW2 + 1) % BUF_DERIV, k = 0; k < tamW2; k++, i = (i + 1) % BUF_DERIV)
      memcpy (Vec_CC[k], buffCC[i], sizeof (float) * n_cmp3);

    return (tamW2);
  }
}

/** @} */ // end of derivative
