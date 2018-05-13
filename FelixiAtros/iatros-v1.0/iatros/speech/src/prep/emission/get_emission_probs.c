#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <unistd.h>
#include <getopt.h>
#include <prep/lda/lda.h>
#include <iatros/hmm.h>
#include <prhlt/gzip.h>
#include <prhlt/constants.h>
#include <prhlt/trace.h>
#include <prhlt/utils.h>
#include <iatros/features.h>

void help() {
  printf("Usage of the get_emission_probs program (iAtros)\n");
  printf("This software computes the emission probabilities of a CC file\n");
  printf("-h\t This help\n");
  printf("-n\t Number of features\n");
  printf("-m\t HMM Model File\n");
  printf("-i\t Input File\n");
  printf("-o\t Output File\n");

}

int main(int argc, char *argv[]) {

  char *sInputFile = NULL;
  char *sOutputFile = NULL;
  char *hmm_fn = NULL;
  int iCCs = 0;

  int option;
  while ((option = getopt(argc, argv, "i:m:o:n:dh")) != -1) {
    switch (option) {
    case 'i':
      sInputFile = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
      strcpy(sInputFile, optarg);
      break;
    case 'm':
      hmm_fn = (char *) malloc(sizeof(char) * strlen(optarg) + 1);
      strcpy(hmm_fn, optarg);
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


  FILE *hmm_file = gzopen(hmm_fn, "r");
  CHECK_SYS_ERROR(hmm_file != NULL, "Couldn't open hmm file '%s'\n", hmm_fn);

  hmm_t *hmm = hmm_create();
  hmm_load(hmm, hmm_file);
  gzclose(hmm_file);


  features_t *feas = features_create_from_file(sInputFile);

  // for each class
  for (int t = 0; t < feas->n_vectors; t++) {
    float *probs = (float *)malloc(hmm->num_states * sizeof(float));
    MEMTEST(probs);
    for (int s = 0; s < hmm->num_states; s++) {
      probs[s] = log_gaussian_mixture(feas->vector[t], hmm->states[s]->mixture, hmm->num_features);
    }
    SWAP(probs, feas->vector[t], float *);
    free(probs);
  }
  feas->n_features = hmm->num_states;

  FILE *fileCCout = gzopen(sOutputFile, "w");

  features_save(feas, fileCCout);
  features_delete(feas);

  gzclose(fileCCout);

  hmm_delete(hmm);
  free(sInputFile);
  free(sOutputFile);
  free(hmm_fn);

}
