#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <prep/libAudio.h>

#define VALUE_AUX 10000

/**
   Circular buffer of audio

 */
typedef struct {
  short int *data;  /*!< Data of the buffer*/
  int forWrite;  /*!< Indicates the next value that can be writen*/
  int forRead; /*!< Indicates the next value that can be read*/
  int numElements; /*!< Max elements in the buffer */
  unsigned int numBytes; /*!< Bytes stored in the buffer*/
  int valueAux; /*!< If this values changes, the buffer will not receive new data */
  int bytesRead; /*!< Bytes read*/
  int bytesWrite; /*!< Bytes writen*/
} specialBuffer;

/**
   Circular buffer of CCs

 */
typedef struct {
  float **data;  /*!< Data of the buffer*/
  int forWrite; /*!< Indicates the next value that can be writen*/
  int forRead; /*!< Indicates the next value that can be read*/
  int numElements;  /*!< Max elements in the buffer */
  unsigned int numCC; /*!< CCs stored in the buffer */
  int valueAux; /*!< If this values changes, the buffer will not receive new data */
} specialBufferVec;

specialBuffer *createBuffer(int, float);
int clearBuffer(specialBuffer *, float);
specialBufferVec *createBufferVec(int , int );
int *clearBufferVec(specialBufferVec *);
int writeBuffer(specialBuffer *, short int *, int );
int writeBufferVec(specialBufferVec *, float *, int );
int readFromBuffer(specialBuffer *, short int *, feS *);
int readFromBufferLast(specialBuffer *, short int *, feS *);
int readFromBufferVec(specialBufferVec *, float *, int );
int freeBuffer(specialBuffer *);
int freeBufferVec(specialBufferVec *);

#endif
