#ifndef LIBPREP_H
#define LIBPREP_H

#include "libpgm.h"
#define UMBRAL_VERTICAL 1
#define SEPARACION_MIN_LETRAS 25  /* en normtamanyo era 10 */

#ifndef MIN
#define MIN(x,y) ( ((x)<(y)) ? (x) : (y) )
#endif
#ifndef MAX
#define MAX(x,y) ( ((x)>(y)) ? (x) : (y) )
#endif

int * getV_Projection(gray **img,int rows,int cols, gray imaxval);
int tamanyo_medio_huecos(int * V_Projection, int cols);
int deteccionTramos(gray ** img, int ** inicio_tramo, int ** fin_tramo,int rows, int cols, gray imaxval);
void copiar(gray **origen, gray **destino,int cols, int rows) ;
void trasladarpgm(gray **origen, gray **destino, int col_inicial, int col_final, int incy, int rows);
gray ** centrarLinBase(gray ** img, int n_tramos, int *inicio, int *fin, int * cogxlw, int rows, int cols);
gray ** crop(gray ** img,int cols, int rows, int *rows_crop, int *cols_crop, gray imaxval);
gray ** row_crop(gray ** img,int cols, int rows, int *rows_crop, gray imaxval);
#endif

