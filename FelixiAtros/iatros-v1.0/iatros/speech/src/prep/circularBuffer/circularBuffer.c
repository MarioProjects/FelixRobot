/** @defgroup circular Circular Buffer
 *  @ingroup prep
 *  Two circular buffers. For audio and cepstrals
 *  @{
 */

/** @page authorsCir Authors
 *  Vicent Tamarit - vtamarit@iti.upv.es
 *  February 2008  
 */

#include "circularBuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// Creates a new circular buffer for audio
/**
   @param[in] numElements Max elements int the buffer
   @param[in] fSilenceThreshold The value used for the initalization of the elements
   @return output pointer to a new circular buffer.
*/
specialBuffer *createBuffer(int numElements, float fSilenceThreshold){
  
  int i;
  specialBuffer *output;
  output=(specialBuffer *)malloc(sizeof(specialBuffer));
  output->numElements=numElements;
  output->forWrite=0;
  output->forRead=0;
  output->numBytes=0;
  output->valueAux=VALUE_AUX;
  output->data=(short int *)malloc(sizeof(short int)*output->numElements);
  for(i=0;i<output->numElements;i++){
    output->data[i]=(short int)fSilenceThreshold;
  }
  output->bytesRead=0;
  output->bytesWrite=0;
  return output;
}


/// Clear an audio circular buffer
/**
   @param[in] output Pointer to an audio circular buffer
   @param[in] fSilenceThreshold The value used for the initalization of the elements
   @return 0 if there is no problem
*/
int clearBuffer(specialBuffer *output, float fSilenceThreshold){

  int i;
  output->forWrite=0;
  output->forRead=0;
  output->numBytes=0;
  output->valueAux=VALUE_AUX;
  output->bytesRead=0;
  output->bytesWrite=0;
  for(i=0;i<output->numElements;i++){
    output->data[i]=(short int)fSilenceThreshold;
  }

  return 0;
}

/// Creates a new circular buffer for CC
/**
   @param[in] numElements Max elements int the buffer
   @param[in] iCCs Each vector in the buffer will be of iCCs elements
   @return output pointer to a new circular buffer.
*/
specialBufferVec *createBufferVec(int numElements, int iCCs){
  
  int i;
  specialBufferVec *output;
  output=(specialBufferVec *)malloc(sizeof(specialBufferVec));
  output->numElements=numElements;
  output->data=(float **)malloc(sizeof(float *)*output->numElements);
  for(i=0;i<output->numElements;i++)
    output->data[i]=(float *)malloc(sizeof(float)*(iCCs)*3);
  output->forWrite=0;
  output->forRead=0;
  output->numCC=0;
  output->valueAux=VALUE_AUX;
 
  return output;
}

/// Clear a CC circular buffer
/**
   @param[in] output Pointer to an CC circular buffer
   @return 0 if there is no problem
*/
int *clearBufferVec(specialBufferVec *output){
  
  output->forWrite=0;
  output->forRead=0;
  output->numCC=0;
  output->valueAux=VALUE_AUX;
 
  return 0;
}


/// Use with on-line recognition
/**
   This functions takes a line of the signal captured and writes it into the circular buffer
   @param[out] circularBuffer The circular buffer. This buffer is shared with the process thread
   @param[in] buffer The line buffer read
   @param[in] sizeBuffer SIze of the buffer vector

   @return 0 if there is no problem
*/
int writeBuffer(specialBuffer *circularBuffer, short int *buffer, int sizeBuffer){

  int i, next;

  next=circularBuffer->forWrite;

  for(i=0;i<sizeBuffer;i++){
    
    circularBuffer->data[next]=buffer[i];
    circularBuffer->numBytes++;

    next=(next+1)%circularBuffer->numElements;
  }

  circularBuffer->forWrite=next;
  circularBuffer->bytesWrite+=sizeBuffer;

  return 0;

}

/// Use with on-line recognition
/**
   This functions takes a vector of CC and writes it into the circular buffer
   @param[out] circularBufferCC The circular buffer. This buffer is shared with the recog thread
   @param[in] CC The new vector
   @param[in] sizeCC Length of the CC vector

   @return 0 if there is no problem
*/
int writeBufferVec(specialBufferVec *circularBufferCC, float *CC, int sizeCC){

  int next;      
  next=circularBufferCC->forWrite;
  memcpy(circularBufferCC->data[next], CC, sizeCC*sizeof(float));
  
  circularBufferCC->numCC++;
  next=(next+1)%circularBufferCC->numElements;
  circularBufferCC->forWrite=next;
  return 0;

}

/// Read data from an audio circular buffer
/**
   @param[in] circularBuffer The circular buffer.
   @param[out] output The data read
   @param[in] FE Feature structure.

   @return 0 if there is no problem
*/
int readFromBuffer(specialBuffer *circularBuffer, short int *output, feS *FE){
  
  int i, next;

  next=circularBuffer->forRead;
  
  for(i=0;i<FE->iFrameSize;i++){
    
    output[i]=circularBuffer->data[next];
    
    next=(next+1)%circularBuffer->numElements;
  }
  circularBuffer->numBytes-=FE->iFrameShift;//circularBuffer->numBytes-FE->iFrameSize+FE->iFrameShift;
  
  circularBuffer->forRead=(circularBuffer->forRead+FE->iFrameShift)%circularBuffer->numElements;

  circularBuffer->bytesRead+=FE->iFrameShift;
  return 0;
}

/// Read the last data from an audio circular buffer
/**
   @param[in] circularBuffer The circular buffer.
   @param[out] output The data read
   @param[in] FE Feature structure.

   @return 0 if there is no problem
*/
int readFromBufferLast(specialBuffer *circularBuffer, short int *output, feS *FE){

  int i, iterFor, next;

  next=circularBuffer->forRead;
  iterFor=circularBuffer->numBytes;
  for(i=0;i<iterFor;i++){

    output[i]=circularBuffer->data[next];
    circularBuffer->numBytes--;
    
    next=(next+1)%circularBuffer->numElements;
    
  }
  
  for(;i<FE->iFrameSize;i++){
    output[i]=circularBuffer->valueAux;
  }
  circularBuffer->bytesRead+=iterFor;
  return 0;
  
}

/// Read one vecotr from a CC circular buffer
/**
   @param[in] circularBufferCC The circular buffer.
   @param[out] vector_cc The vector read
   @param[in] iCCs LEntgh of the Cc vector

   @return 0 if there is no problem
*/

int readFromBufferVec(specialBufferVec *circularBufferCC, float *vector_cc, int iCCs){
  
  int next;

  next=circularBufferCC->forRead;
  
  memcpy(vector_cc, circularBufferCC->data[next], sizeof(float)*(iCCs)*3 );
  
  next=(next+1)%circularBufferCC->numElements;
  circularBufferCC->numCC--;
  circularBufferCC->forRead=next;

  return 0;
}

/// Free memory of an audio circular buffer
/**
   @param[out] circularBuffer The circular buffer.
   @return 0 if there is no problem
*/
int freeBuffer(specialBuffer *circularBuffer){

  free(circularBuffer->data);
  free(circularBuffer);

  return 0;
}

/// Free memory of a CC circular buffer
/**
   @param[out] circularBufferCC The circular buffer.
   @return 0 if there is no problem
*/
int freeBufferVec(specialBufferVec *circularBufferCC){

 int i;

  for(i=0;i<circularBufferCC->numElements;i++)
    free(circularBufferCC->data[i]);;

  free(circularBufferCC->data);
  free(circularBufferCC);

  return 0;
}


/** @} */ // end of circular
