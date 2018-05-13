/*
 * acquisitor.c
 *
 *  Created on: 29-sep-2008
 *      Author: valabau
 */

#include <prep/acquisitor.h>
#include <prep/libAudio.h>
#include <prep/circularBuffer/circularBuffer.h>
#include <prhlt/trace.h>
#include <pthread.h>

struct acquisitor_t {
  hrw *hrwParam;
  bf **filter;
  feS *FE;

  sound *record;
  specialBuffer *circularBuffer;
  specialBufferVec *circularBufferCC;

  pthread_mutex_t mutexBufferVec;

  pthread_mutex_t condition_online_mutex;
  pthread_cond_t condition_online_cond;

  pthread_t threadSound;
  pthread_t threadProcess;

  int iUse;
  void *argTS[5];
  void *argTP[8];

  int bNonStop;
};


acquisitor_t *acquisitor_create() {

  acquisitor_t *acquisitor = (acquisitor_t *)malloc(sizeof(acquisitor_t));
  MEMTEST(acquisitor);
  acquisitor->hrwParam = (hrw *)malloc(sizeof(hrw));
  MEMTEST(acquisitor->hrwParam);

  return acquisitor;
}

acquisitor_t *acquisitor_create_from_file(const char *filename) {
  acquisitor_t *acquisitor = acquisitor_create();
  acquisitor->FE=createFE();

  /*Load Configuration from file*/
  loadConfiguration((acquisitor->FE), &(acquisitor->filter), acquisitor->hrwParam, filename);

  acquisitor->circularBuffer = createBuffer(acquisitor->FE->iSizeSignalBuffer,acquisitor->FE->fSilenceThreshold);
  acquisitor->circularBufferCC = createBufferVec(acquisitor->FE->iSizeCCBuffer, acquisitor->FE->iCCs);

  pthread_mutex_init(&acquisitor->mutexBufferVec, NULL);

  pthread_mutex_init(&acquisitor->condition_online_mutex, NULL);
  pthread_cond_init(&acquisitor->condition_online_cond, NULL);

  acquisitor->record = createSound();
  acquisitor->record->buffer = (short int *) calloc(acquisitor->FE->iFrameSize, sizeof(short int));

  acquisitor->iUse = 0;

  acquisitor->argTS[0] = (void *) acquisitor->hrwParam;
  acquisitor->argTS[1] = (void *) acquisitor->circularBuffer;
  acquisitor->argTS[2] = (void *) acquisitor->record;
  acquisitor->argTS[3] = (void *) &acquisitor->iUse;
  acquisitor->argTS[4] = (void *) acquisitor->FE;

  acquisitor->argTP[0] = (void *) acquisitor->filter;
  acquisitor->argTP[1] = (void *) acquisitor->circularBuffer;
  acquisitor->argTP[2] = (void *) acquisitor->FE;
  acquisitor->argTP[3] = (void *) acquisitor->circularBufferCC;
  acquisitor->argTP[4] = (void *) acquisitor->record;
  acquisitor->argTP[5] = (void *) &acquisitor->mutexBufferVec;
  acquisitor->argTP[6] = (void *) &acquisitor->condition_online_mutex;
  acquisitor->argTP[7] = (void *) &acquisitor->condition_online_cond;

  return acquisitor;
}

void acquisitor_delete(acquisitor_t *acquisitor) {
  free(acquisitor->record->buffer);
  free(acquisitor->record);
  freeBuffer(acquisitor->circularBuffer);
  freeBufferVec(acquisitor->circularBufferCC);

  pthread_mutex_destroy(&acquisitor->mutexBufferVec);
}

void acquisitor_start(acquisitor_t *acquisitor) {
  if (pthread_create(&acquisitor->threadSound, NULL, (void *(*)(void *)) readBuffer, acquisitor->argTS)) {
    fprintf(stderr, "error creating thread.");
    abort();
  }

  if (pthread_create(&acquisitor->threadProcess, NULL, (void *(*)(void *))processBuffer, (void *) acquisitor->argTP)) {
    fprintf(stderr, "error creating thread.");
    abort();
  }
  acquisitor->bNonStop = 1;
}

void acquisitor_get_next(acquisitor_t *acquisitor, float *vector_cc) {

  pthread_mutex_lock(&acquisitor->condition_online_mutex);
  if (acquisitor->circularBufferCC->numCC <= 0
      && acquisitor->circularBufferCC->valueAux == VALUE_AUX)
  {
    pthread_cond_wait(&acquisitor->condition_online_cond, &acquisitor->condition_online_mutex);
    pthread_mutex_unlock(&acquisitor->condition_online_mutex);
  } else {
    pthread_mutex_unlock(&acquisitor->condition_online_mutex);
  }

  pthread_mutex_lock(&acquisitor->mutexBufferVec);
  readFromBufferVec(acquisitor->circularBufferCC, vector_cc, acquisitor->FE->iCCs);
  pthread_mutex_unlock(&acquisitor->mutexBufferVec);

  if (acquisitor->circularBufferCC->valueAux != VALUE_AUX
      && acquisitor->circularBufferCC->numCC == 0)
  { //The last buffer
    acquisitor->bNonStop = 0;
    if (pthread_join(acquisitor->threadSound, NULL)) {
      fprintf(stderr, "error joining thread.");
      abort();
    }

    if (pthread_join(acquisitor->threadProcess, NULL)) {
      printf("error joining thread.");
      abort();
    }
  }
}

int acquisitor_has_next(const acquisitor_t *acquisitor) {
  return acquisitor->bNonStop;
}

int acquisitor_num_features(const acquisitor_t *acquisitor) {
  return (acquisitor->FE->iCCs) * 3;
}

void acquisitor_clear(acquisitor_t *acquisitor) {
  clearSound(acquisitor->record);
  acquisitor->record->buffer = (short int *) calloc(acquisitor->FE->iFrameSize, sizeof(short int));
  clearBuffer(acquisitor->circularBuffer, acquisitor->FE->fSilenceThreshold);
  clearBufferVec(acquisitor->circularBufferCC);
}
