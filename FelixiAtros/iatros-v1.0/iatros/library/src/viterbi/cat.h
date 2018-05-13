/*
 * cat.h
 *
 *  Created on: 17-feb-2009
 *      Author: valabau
 */

#ifndef CAT_H_
#define CAT_H_

#include <iatros/viterbi.h>

typedef enum { REF_NONE, REF_SOURCE, REF_TARGET, REF_MAX } reference_t;

grammar_t * grammar_create_from_prefix(const grammar_t *grammar, const symbol_t *input_prefix, const symbol_t *output_prefix);

search_t *search_create_from_prefix(const search_t *search, symbol_t *in_prefix, symbol_t *out_prefix);

int cat_decode(search_t *search, const features_t *features, lattice_t *lattice,
               symbol_t *ref, reference_t ref_type);


#endif /* CAT_H_ */
