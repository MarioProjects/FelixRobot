#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <unistd.h>
#include <getopt.h>
#include <prep/derivative/derivative.h>
#include <prhlt/gzip.h>
#include <prhlt/constants.h>
#include <prhlt/trace.h>
#include <iatros/features.h>

void help() {
  printf("Usage of the get_deriv program (iAtros)\n");
  printf("This software computes the first and second derivtive of a CC file\n");
  printf("-h\t This help\n");
  printf("-i\t Input File \n");
  printf("-o\t Output File (default stdout)\n");
  printf("-a\t Aachen derivatives\n");

}

int main(int argc, char *argv[]) {

  char *sInputFile = NULL;
  char *sOutputFile = NULL;

  int option;
  deriv_t type = HTK;

  while ((option = getopt(argc, argv, "i:o:ah")) != -1) {
    switch (option) {
    case 'i':
      sInputFile = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
      strcpy(sInputFile, optarg);
      break;
    case 'o':
      sOutputFile = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
      strcpy(sOutputFile, optarg);
      break;
    case 'a':
      type = AACHEN;
      break;
    case 'h':
      help();
      break;
    default:
      break;
    }
  }//End while

  if(sInputFile==NULL){
    fprintf(stderr,"ERROR: There is no input file !!\n\n");
    help();
    exit(1);
  }


  features_t *feas = features_create_from_file(sInputFile);
  compute_derivatives(feas, type);

  FILE * fileCCout;
  if(sOutputFile==NULL){
    fileCCout=stdout;
  }
  else{
    fileCCout=gzopen(sOutputFile, "w");
  }

  features_save(feas, fileCCout);
  gzclose(fileCCout);

  features_delete(feas);

  free(sInputFile);
  free(sOutputFile);
}
