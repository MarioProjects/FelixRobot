/*
 * acquisitor.h
 *
 *  Created on: 29-sep-2008
 *      Author: valabau
 */

#ifndef ACQUISITOR_H_
#define ACQUISITOR_H_

typedef struct acquisitor_t acquisitor_t;

acquisitor_t *acquisitor_create_from_file(const char *filename);
void acquisitor_delete(acquisitor_t *acquisitor);
void acquisitor_start(acquisitor_t *acquisitor);
void acquisitor_get_next(acquisitor_t *acquisitor, float *vector);
int acquisitor_has_next(const acquisitor_t *acquisitor);
int acquisitor_num_features(const acquisitor_t *acquisitor);
void acquisitor_clear(acquisitor_t *acquisitor);

#endif /* ACQUISITOR_H_ */
