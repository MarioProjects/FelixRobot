/* pgm.h - header file for libpgm portable graymap library
*/
#include <stdio.h>

#ifndef _PGM_H_
#define _PGM_H_


/* The following definition has nothing to do with the format of a PGM file */
typedef unsigned char gray;

#define PGM_MAXVAL 255


/* Magic constants. */

#define PGM_MAGIC1 'P'
#define PGM_MAGIC2 '2'
#define RPGM_MAGIC2 '5'
#define PGM_FORMAT (PGM_MAGIC1 * 256 + PGM_MAGIC2)
#define RPGM_FORMAT (PGM_MAGIC1 * 256 + RPGM_MAGIC2)
#define PGM_TYPE PGM_FORMAT
#define UNK_FORMAT -1

/* Macro for turning a format number into a type number. */

#define PGM_FORMAT_TYPE(f) ((f) == PGM_FORMAT || (f) == RPGM_FORMAT ? PGM_TYPE : UNK_FORMAT -1)


/* Declarations of routines. */

gray ** pgm_allocarray( int cols, int rows );

void pgm_freearray(gray ** const rowIndex, int const rows);

gray** pgm_readpgm ( FILE* file, int* colsP, int* rowsP, gray* maxvalP );

void pgm_writepgm ( FILE* file, gray** grays, int cols, int rows, gray maxval, int forceplain );

#endif /*_PGM_H_*/
