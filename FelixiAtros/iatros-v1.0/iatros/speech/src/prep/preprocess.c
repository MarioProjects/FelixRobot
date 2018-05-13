#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <unistd.h>
#include <getopt.h>
#include <prep/derivative/derivative.h>
#include <prep/lda/lda.h>
#include <iatros/hmm.h>
#include <prhlt/gzip.h>
#include <prhlt/constants.h>
#include <prhlt/trace.h>
#include <prhlt/utils.h>
#include <iatros/features.h>

void help() {
  printf("Usage of the preprocess program (iAtros)\n");
  printf("This software preprocesses a CC file\n");
  printf("-h\t This help\n");
  printf("-i <file>\t Input file\n");
  printf("-o <file>\t Output file\n");
  printf("-d\t HTK derivatives\n");
  printf("-a\t Aachen derivatives\n");
  printf("-l <file>\t LDA matrix file\n");
  printf("-m <file>\t HMM file\n");
  printf("-b\t batch mode. Takes input file as a list of files, and output file as an extension to be append to the filename\n");
}

typedef struct {
  deriv_t type;
  bool is_batch;
  char *lda_fn;
  lda_t *lda;
  char *hmm_fn;
  hmm_t *hmm;
  int n_features;
} preprocess_t;


void preprocess_file(const char *input_fn, const char *output_fn, preprocess_t * preprocess) {

  FILE *fileCCout = NULL;

  features_t * feas = features_create_from_file(input_fn);

  if (preprocess->n_features == 0) {
    preprocess->n_features = feas->n_features;
  }
  else {
    CHECK(preprocess->n_features == feas->n_features, "CC differs with previous CCs in number of features\n");
  }



  if (preprocess->type != NO_DERIV) {
    compute_derivatives(feas, preprocess->type);
  }


  if (preprocess->lda_fn) {
    if (preprocess->lda == NULL) {
      preprocess->lda = lda_create();
      lda_load(preprocess->lda, preprocess->lda_fn, feas->n_features);
    }
    lda_transform(preprocess->lda, feas);
  }

  if (preprocess->hmm_fn) {
    if (preprocess->hmm == NULL) {
      FILE *hmm_file = gzopen(preprocess->hmm_fn, "r");
      CHECK_SYS_ERROR(hmm_file != NULL, "Couldn't open hmm file '%s'\n", preprocess->hmm_fn);

      preprocess->hmm = hmm_create();
      hmm_load(preprocess->hmm, hmm_file);
      gzclose(hmm_file);
    }

    CHECK(preprocess->hmm->num_features == feas->n_features, "Number of features mismatch\n");
    hmm_compute_emission_probabilities(preprocess->hmm, feas);
  }

  fileCCout = gzopen(output_fn, "w");
  features_save(feas, fileCCout);
  gzclose(fileCCout);

  features_delete(feas);
}

int main(int argc, char *argv[]) {

  char *input_fn = NULL;
  char *output = NULL;
  preprocess_t preprocess = { NO_DERIV, false, NULL, NULL, NULL, NULL, 0 };

  int option;
  while ((option = getopt(argc, argv, "i:o:dal:m:bh")) != -1) {
    switch (option) {
    case 'i':
      input_fn = optarg;
      break;
    case 'o':
      output = optarg;
      break;
    case 'd':
      preprocess.type = HTK;
      break;
    case 'a':
      preprocess.type = AACHEN;
      break;
    case 'l':
      preprocess.lda_fn = optarg;
      break;
    case 'm':
      preprocess.hmm_fn = optarg;
      break;
    case 'b':
      preprocess.is_batch = true;
      break;
    case 'h':
      help();
      break;
    default:
      break;
    }
  }//End while

  if (preprocess.is_batch) {
    //Open file of cepstrals
    FILE *file_cepstrals = gzopen(input_fn, "r");
    CHECK_SYS_ERROR(file_cepstrals != NULL, "Couldn't open file of feature vectors '%s'\n", input_fn);

    //for each and every sample
    char line[MAX_LINE];
    char output_fn[MAX_LINE];
    while (fgets(line, MAX_LINE, file_cepstrals) != 0) {
      line[strlen(line) - 1] = '\0';
      strcpy(output_fn, line);
      strcat(output_fn, output);
      fprintf(stderr, "Preprocessing '%s' -> '%s'\n", line, output_fn);
      preprocess_file(line, output_fn, &preprocess);
    }
  }
  else {
    preprocess_file(input_fn, output, &preprocess);
  }

  if (preprocess.lda) lda_delete(preprocess.lda);
  if (preprocess.hmm) hmm_delete(preprocess.hmm);
}
