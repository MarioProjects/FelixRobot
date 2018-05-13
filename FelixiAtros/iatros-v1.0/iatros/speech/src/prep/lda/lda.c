/** @defgroup Lda Lda Module
 *  @ingroup prep
 *  Lda fuction
 *  @{
 */

/** @page authors Authors
 *
 *       Authors:
 *              Moises Pastor (original version in ATROS)\n
 *              Vicent Alabau (adaptation of the source to iATROS)\n
 */

#include <prep/lda/lda.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <prhlt/gzip.h>
#include <prhlt/trace.h>
#include <prhlt/constants.h>

#ifdef MAX_LINE
#undef MAX_LINE
#endif

#define MAX_LINE (1024*1024)
#define MAX_VALUES 256

///Loads lda matrix from file
/**
  @param lda lda matrix to load
  @param[in] filename path to lda file
  @param[in] n_ceps number of cepstrals
 */
int lda_load(lda_t *lda, const char *filename, int n_ceps) {
  //Line of file
  char line[MAX_LINE];
  //File of lda
  FILE * lda_file;

  //Open lda
  lda_file = gzopen(filename,"r");
  CHECK_SYS_ERROR(lda_file != NULL, "No lda file '%s'\n", filename);

  //Read lda
  // Read first line
  int num_rows = 0;
  {
    char *startp, *endp = NULL;
    double values[MAX_VALUES], value;
    int num_cols = 0;
    fgets(line, MAX_LINE, lda_file);
    startp = line;
    value = strtod(startp, &endp);
    while (startp != endp) {
      values[num_cols++] = value;
      startp = endp;
      value = strtod(startp, &endp);
    }
    REQUIRE(num_cols % n_ceps == 0, "Size of lda matrix (%d) and feature vectors (%d) do not match\n", num_cols, n_ceps);
    lda->window_width = num_cols / n_ceps;
    lda->size = num_cols;
    lda->matrix = (double **) malloc(num_cols * sizeof(double *));
    for (int i = 0; i < num_cols; i++) {
      lda->matrix[i] = (double *) malloc(num_cols * sizeof(double));
    }
    memcpy(lda->matrix[num_rows], values, num_cols * sizeof(double));
    num_rows++;
  }
  // read remainig values
  while(fgets(line, MAX_LINE, lda_file) != 0) {
    char *startp, *endp = NULL;
    int num_cols = 0;
    double value;
    startp = line;
    value = strtod(startp, &endp);
    while (startp != endp) {
      lda->matrix[num_rows][num_cols++] = value;
      startp = endp;
      value = strtod(startp, &endp);
    }
    REQUIRE(num_cols == lda->size, "Invalid number of elements in line %d (%d vs %d)\n", num_rows + 1, num_cols, lda->size);
    num_rows++;
  }

  REQUIRE(num_rows == lda->size, "Invalid number of rows\n");

  gzclose(lda_file);
  return 1;
}

lda_t * lda_create() {
  return (lda_t *)calloc(1, sizeof(lda_t));
}

///
///deletes the lda
/**
  @param lda lda to delete
 */
void lda_delete(lda_t * lda) {
  for (int i = 0; i < lda->size; i++) {
    free(lda->matrix[i]);
  }
  free(lda->matrix);
  free(lda);
}

/// Computes the lda transformation of the feature vectors
/**
  @param lda the lda matrix
  @param feat_vectors a list of feature vectors
  @param[in] size the number of feature_vectors
  @return the number of feature vectors after the transformation
  The new transformed feature vectors are stored in feat_vector and thus
  erasing previous information. If that information should be kept a copy
  of feat_vectors will be necessary.
*/
void lda_transform(const lda_t *lda, features_t *features) {

  int n_ceps = lda->size / lda->window_width;
  int new_size = features->n_vectors - lda->window_width + 1;
  float res[n_ceps];
  for (int pos = 0; pos < new_size; pos++) {
    for (int cep = 0; cep < n_ceps; cep++) {
      res[cep] = 0;
      for (int w = 0; w < lda->size; w++) {
        res[cep] += lda->matrix[w][cep] * features->vector[pos + (w/n_ceps)][w%n_ceps];
      }
    }
    memcpy(features->vector[pos], res, n_ceps * sizeof(float));
  }

  // fill structure field
  char *structure = (char *)malloc(5*MAX_LINE*sizeof(char));
  strcpy(structure, "");
  for (int i = 0; i < features->n_features; i++) {
    sprintf(structure, "%s lda%d", structure, i);
  }
  features->structure = (char *)realloc(features->structure, (strlen(structure)+1)*sizeof(char));
  strcpy(features->structure, structure);
  free(structure);

  features_resize(features, new_size);
  features->type = FF_LDA;
}

/** @} */ // end of lda
