#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <unistd.h>
#include <getopt.h>
#include <prep/lda/lda.h>
#include <prhlt/gzip.h>
#include <prhlt/constants.h>
#include <prhlt/trace.h>
#include <iatros/features.h>

void help() {
  printf("Usage of the get_lda program (iAtros)\n");
  printf("This software computes the lda transformation of a CC file\n");
  printf("-h\t This help\n");
  printf("-n\t Number of features\n");
  printf("-m\t LDA Matrix File\n");
  printf("-i\t Input File\n");
  printf("-o\t Output File\n");

}

int main(int argc, char *argv[]) {

  char *sInputFile = NULL;

  char *sOutputFile = NULL;
  char *ldaMatrixFile = NULL;
  FILE *fileCCout = NULL;
  int iCCs = 0;

  int option;

  while ((option = getopt(argc, argv, "i:m:o:n:dh")) != -1) {
    switch (option) {
    case 'i':
      sInputFile = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
      strcpy(sInputFile, optarg);
      break;
    case 'm':
      ldaMatrixFile = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
      strcpy(ldaMatrixFile, optarg);
      break;
    case 'o':
      sOutputFile = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
      strcpy(sOutputFile, optarg);
      break;
    case 'n':
      iCCs = atoi(optarg);
      break;
    case 'h':
      help();
      break;
    default:
      break;
    }
  }//End while



  features_t *feas = features_create_from_file(sInputFile);

  lda_t lda;
  lda_load(&lda, ldaMatrixFile, iCCs);

  lda_transform(&lda, feas);

  fileCCout = gzopen(sOutputFile, "w");
  features_save(feas, fileCCout);
  gzclose(fileCCout);

  lda_delete(&lda);
  free(sInputFile);
  free(sOutputFile);
  free(ldaMatrixFile);

}
