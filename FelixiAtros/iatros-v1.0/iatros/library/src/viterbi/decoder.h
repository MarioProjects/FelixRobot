/*
 * decoder.h
 *
 *  Created on: 19-abr-2009
 *      Author: valabau
 */

#ifndef DECODER_H_
#define DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <iatros/grammar.h>
#include <prhlt/args.h>

#define DECODER_MODULE_NAME "decoder"

static const arg_module_t decoder_module = { DECODER_MODULE_NAME, "decoder module",
    {
        {"hmm", ARG_FILE, NULL, 0, "Hidden Markov Model in HTK format"},
        {"lexicon", ARG_FILE, NULL, 0, "Lexicon file"},
        {"lexicon-type", ARG_STRING, "ATROS", 0, "Lexicon format (ATROS, HTK)"},
        {"grammar", ARG_FILE, NULL, 0, "Main grammar"},
        {"grammar-type", ARG_STRING, "NGRAM", 0, "Type of the main grammar (FSM, NGRAM). NGRAM by default"},
        {"input-grammar", ARG_FILE, NULL, 0, "Input grammar"},
        {"output-grammar", ARG_FILE, NULL, 0, "Output grammar"},

        {"beam", ARG_FLOAT, "1e+30", 0, "Relative beam"},
        {"grammar-scale-factor", ARG_FLOAT, "1", 0, "Grammar scale factor"},
        {"input-grammar-scale_factor", ARG_FLOAT, "1", 0, "Grammar scale factor for the input grammar"},
        {"output-grammar-scale-factor", ARG_FLOAT, "1", 0, "Grammar scale factor for the output grammar"},
        {"word-insertion-penalty", ARG_FLOAT, "0", 0, "Word insertion penalty"},
        {"output-word-insertion-penalty", ARG_FLOAT, "0", 0, "Word insertion penalty for output words"},

        {"start", ARG_STRING, NULL, 0, "Start symbol. None by default"},
        {"end", ARG_STRING, NULL, 0, "End symbole. None by default"},
        {"unk", ARG_STRING, NULL, 0, "Unknown symbol. None by default"},
        {"silence", ARG_STRING, NULL, 0, "Silence symbol. None by default"},
        {"silence-score", ARG_FLOAT, "1.0", 0, "Score for the silence word. '1.0' by default"},

        {"histogram-pruning", ARG_INT, "10000", 0, "Maximum number of hypotheses allowed by frame."},

        {"phrase-table", ARG_FILE, NULL, 0, "Phrase table in moses format"},
        {"weights", ARG_STRING, NULL, 0, "Weights for the log-lineal model in the phrase table separated by commas"},

        {"do-acoustic-early-pruning", ARG_BOOL, "true", 0, "Enables acoustic early pruning"},
        {"create-dummy-acoustic-models", ARG_BOOL, NULL, 0, "Enables the creation of dummy acoustic models"},

        {"categories", ARG_FILE, NULL, 0, "List of the categories with the associated grammars"},
        {NULL, ARG_END_MODULE, NULL, 0, NULL}
    }
};


///Categories
typedef struct categories {
 grammar_t **categories;  ///< Vector of aef with categories
 int num_categories;      ///< Number of categories
} categories_t;

typedef struct {
  extended_vocab_t *vocab;         ///< vocabulary
  hmm_t *hmm;                      ///< Acoustic model
  lex_t *lex;                      ///< Lexical model
  grammar_t *grammar;              ///< Grammar model
  categories_t *categories;          ///< Structure of categories
  grammar_t *input_grammar;        /**< Input language model. To make the search simpler,
                                      * the input grammar must be deterministic at word level.
                                      * Thus, phrase models and fsm are forbidden. Assuming n-gram
                                      */
  grammar_t *output_grammar;       /**< Output language model. This model can be used with
                                      * conditional probability transducers to perform
                                      * integrated phrase-based speech translation.
                                      * To make the search simpler,
                                      * the input grammar must be deterministic at word level.
                                      * Thus, phrase models and fsm are forbidden. Assuming n-gram
                                      */

  float gsf;             ///< Grammar Scale Factor
  float gsf_in;          ///< Gsf for the input grammar
  float gsf_out;         ///< Gsf for the output grammar
  float wip;             ///< Word Insertion Penalty
  float wip_out;         ///< Word Insertion Penalty for output words
  float silence_score;   ///< Silence score
  int histogram_pruning; ///< Maximum number of hypotheses per frame
  int beam_pruning;      ///< Relative pruning w.r.t. the maximum hypothesis
  bool do_acoustic_early_pruning; /**< If the acoustic early pruning is enabled or not.
                                     *  In this mode, before language model expansions,
                                     *  an estimation of the best acoustic score is
                                     *  obtained as the maximum acoustic score computed
                                     *  so far, for the current feature vector. This estimate
                                     *  is used to prune the hypothesis before expanding
                                     *  the word */

} decoder_t;

decoder_t * decoder_create_from_file(FILE *file);
decoder_t * decoder_create_from_args(const args_t *args);
decoder_t *decoder_dup(decoder_t *source);
void decoder_delete(decoder_t *decoder);

#ifdef __cplusplus
}
#endif

#endif /* DECODER_H_ */
