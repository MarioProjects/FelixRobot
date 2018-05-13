 /*! \author
 Vicent Tamarit - vtamarit@iti.upv.es
 Miriam Lujan Mares mlujan@iti.upv.es
 *  \version 1.0
 *  \date    2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>

#include <iatros-speech/version.h>
#include <iatros/version.h>
#include <prhlt/args.h>
#include <iatros/lex.h>
#include <iatros/hmm.h>
#include <iatros/grammar.h>
#include <iatros/viterbi.h>
#include <iatros/heap.h>
#include <iatros/lattice.h>
#include <prhlt/trace.h>
#include <prhlt/gzip.h>
#include <prhlt/constants.h>
#include <prep/libAudio.h>
#include <prep/acquisitor.h>
#include <time.h>


void outputs(const args_t *args, lattice_t *lattice) {

  //Word_graph
  if (args_get_bool(args, "save-lattices", NULL)) {
    char filename[MAX_LINE];
    char path[MAX_LINE];

    time_t tm = time(NULL);
    strftime(filename, sizeof(filename), "%y-%m-%d_%M:%S", localtime(&tm));
    sprintf(filename, "%s_%d_PID%d", filename, (int)clock(), getpid());

    char const *lattices_dn = args_get_string(args, "lattice-directory", NULL);
    if (lattices_dn == NULL) lattices_dn = ".";
    printf("The word_graph is in %s/%s.lat.gz\n", lattices_dn, filename);

    //Name of file with word_graph
    sprintf(path, "%s/%s.lat.gz", lattices_dn, filename);
    FILE *lattice_file = gzopen(path, "w");
    REQUIRE(lattice_file != NULL, "Couldn't open lattice file '%s'\n", path);
    lattice_write(lattice, lattice_file, path);
    gzclose(lattice_file);
  }

  //Best hypothesis
  symbol_t *sentence;
  float prob = lattice_best_hyp(lattice, &sentence);
  if (sentence != NULL) {
    char *sentence_str = NULL;
    extended_vocab_symbols_to_string(sentence, lattice->decoder->vocab, &sentence_str);
    printf("%f %s\n", prob, sentence_str);
    free(sentence);
    free(sentence_str);
  } else {
    printf("Sentence not recognized\n");
    fflush(stdout);
  }

  //XXX: playSound(acquisitor->hrwParam, acquisitor->record);
}

static const arg_module_t online_module = {NULL, "General options",
    {
      {"save-lattices", ARG_BOOL, "false", 0, "Enables saving lattices"},
      {"lattice-directory", ARG_DIR, NULL, 0, "Directory where lattices will be saved"},
      {"print-time", ARG_BOOL, "false", 0, "Print realtime factor"},
      {"print-score", ARG_BOOL, "false", 0, "Print hypothesis score"},
      {"verbosity", ARG_INT, "0", 0, "Set verbosity level"},
      {"statistics-verbosity", ARG_INT, "0", 0, "Set statistics verbosity level"},
      {"print-default", ARG_BOOL, "false", 0, "Print default config file"},
      {"print-config", ARG_BOOL, "false", 0, "Print final config file"},
      {"preprocess-config", ARG_STRING, NULL, 0, "Preprocess config file"},
      {NULL, ARG_END_MODULE, NULL, 0, NULL}
    }
};

static const arg_shortcut_t shortcuts[] = {
    {"v", "verbosity"},
    {"s", "statistics-verbosity"},
    {"p", "preprocess-config"},
    {"t", "print-time"},
    {"d", "print-default"},
    {"l", "decoder.grammar-scale-factor"},
    {"w", "decoder.word-insertion-penalty"},
    {NULL, NULL}
};


int main (int argc, char *argv[]) {

  arg_error_t aerr = ARG_OK;

  args_t *args = args_create();
  args_set_summary(args, "Online speech text recognizer");
  args_set_doc(args, "For more info see http://prhlt.iti.es");
  args_set_version(args, IATROS_SPEECH_PROJECT_STRING"\n"IATROS_SPEECH_BUILD_INFO"\n\n"
                         IATROS_PROJECT_STRING"\n"IATROS_BUILD_INFO);
  args_set_bug_report(args, "Report bugs to "IATROS_SPEECH_PROJECT_BUGREPORT".");
  args_add_module(args, &online_module);
  args_add_module(args, &decoder_module);
  args_add_module(args, &lattice_module);
  args_add_shortcuts(args, shortcuts);

  args_parse_command_line(args, argc, argv);


  if (args_get_bool(args, "print-default", &aerr)) {
    args_write_default_config_file(args, stdout);
    args_delete(args);
    return EXIT_SUCCESS;
  }

  if (args_get_bool(args, "print-config", &aerr)) {
    args_dump(args, stderr);
  }


  SET_STATISTICS_VERBOSITY(args_get_int(args, "statistics-verbosity", &aerr));

  INIT_TRACE(args_get_int(args, "verbosity", &aerr));

  const char *prep_conf_filename = args_get_string(args, "preprocess-config", &aerr);
  REQUIRE(aerr == ARG_OK && prep_conf_filename != NULL, "Preprocess config file is missing");

  //Load all models
  decoder_t *decoder = decoder_create_from_args(args);

  acquisitor_t *acquisitor = acquisitor_create_from_file(prep_conf_filename);

  //if (1) {
  while (1) {
    //acquisitor_start(acquisitor);

    search_t *search = search_create(decoder);
    lattice_t *lattice = lattice_create_from_args(args, decoder);

    clock_t tim = clock();
    decode_online(search, acquisitor, lattice);

    clock_t tim2 = clock();
    printf("tim %f\n", ((float) ((tim2 - tim) / CLOCKS_PER_SEC) / search->n_frames) / 0.01);
    fflush(stdout);

    outputs(args, lattice);

    lattice_delete(lattice);
    search_delete(search);

    acquisitor_clear(acquisitor);

  } //end while(1)


  decoder_delete(decoder);
  acquisitor_delete(acquisitor);
  args_delete(args);

  return EXIT_SUCCESS;
}
