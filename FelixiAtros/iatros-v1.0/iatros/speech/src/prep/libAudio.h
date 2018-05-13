#ifndef __LIBAUDIO_H__
#define __LIBAUDIO_H__

#include <stdio.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <error.h>
#include <errno.h>
#include <sys/file.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fftw3.h>
#include <iatros-speech/derivative.h>
#include <prhlt/constants.h>

#include <iatros/viterbi.h>
#include <iatros-speech/acquisitor.h>

#define CONFIG_ERROR 1000
#define BUFFER_NOT_INIT 10000
#define HAMMING_NOT_INIT 10001
#define FFT_NOT_INIT 10002
#define FILTER_NOT_INIT 10003
#define NO_FILE_CONFIG 10004
#define NO_MODE_DEFINED 10005
#define NO_FILE_INPUT 10006

#define SIZE_INI 10

/**
   \typedef hrw
   Contains information about the hardware configuration for record/play sounds

 */
typedef struct _hrw{
  int iBits; /*!< Bits for each sample (usually 16)*/
  int iChannels; /*!< Channels 1 Mono, 2 Stereo (usually 1)*/
  unsigned int iSampleRate;/*!< Frecuency (usually 16000Hz)*/
  /**
     Number of frames contained by the soundcard buffer\n
     It seems that 32 frames ought to be enough for anybody
  */
  snd_pcm_uframes_t iFrames;
  char *inputDevice; /*!< Alsa input device. Usually: defualt*/
  char *outputDevice; /*!< Alsa output device. Usually: default*/
} hrw;

/**
   Defines a sound

 */
typedef struct _sound{
  int iBytes; /*!< Bytes of the sound (length of audio vector)*/
  char *audio; /*!< The sound stored in raw format. Used to interact with soundcard.*/
  short int *data; /*!< Sound represented as short integers. Useful for computing.*/
  int iSamples; /*!< Number of samples (length of data vector)*/
  float fTime; /*!< Duration of sound (in seconds).*/
  short int *buffer; /*!< A piece of signal of iFrameSize elements*/
  int lastSample; /*!< The last sample in the buffer*/
  int prior; /*!< Last value of the window (used in preemphasis) */
} sound;


/**
   This structure stores information about the signal thats is useful for the feature extraction

 */
typedef struct {
  int iSampleRate; /*!< Frecuency*/
  int iSubSampleRate; /*!< SubSample Frecuency*/
  int iFFTsize; /*!< Size of the FFT. */
  int iFrameSize; /*!< Size of the frame. One frame -> one CC vector*/
  float fWindowLen; /*!< Length of the window. In seconds*/
  int iFrameShift; /*!< Desplazamiento de la ventana*/
  int nFilters; /*!< Number of filters*/
  int bTriangular; /*!< Indicates the use of triangular filter (1 is Yes)*/
  float fFactor; /*!< Factor of the preemphasis. Usually 0.97*/
  float fWindowAlfa; /*!< Factor of the window. Usually 0.54 (Hamming)*/
  int iCCs; /*!< Number of ceptrals extracted for each frame. Energy included*/
  float fSecondsSilence; /*!< Second of silence that stop the record*/
  float fSilenceThreshold; /*!< Silence Threshold*/
  int iSizeSignalBuffer; /*!< Elements of the buffer for the signal */
  int iSizeCCBuffer; /*!< Elements of the buffer for the CC */
  float **fCosTable; /*!< Cosinus table for the inverse transformation */
  int iDerivative; /*!< Boolean variable that indicates the compute of derivative */
} feS;

/**
   One filter bank

 */
typedef struct _bf{
  float fFrecStart; /*!< Start frequency of the filter */
  float fFrecEnd; /*!< End frequency of the filter */
  int index; /*!< Number of elements within the frequency range (this is the only value we use)*/
} bf;

int nextBuffer(feS*, sound*);
feS *createFE();
sound *createSound();
int clearSound(sound *);
int replicateSound(sound *, sound *);
int calculateFFT(feS *, float *, float *);
int applyFilterBank(feS *, float *, float *, bf **);
int applyTriangularFilterBank(feS *, float *, float *);
int extractCC(feS *FE, float *, float *);
int preemphasis(feS *, short int *, float * , int );
int initHardware(snd_pcm_t *, hrw *, snd_pcm_hw_params_t *);
int playSound(hrw *, sound *);
int readFile(feS *, char *, sound *);
int speechDetection(feS *, sound *, sound *);
int speechDetection2(feS *, sound *, sound *);
int hammingWindow(feS *, float *, float *, float *);
int printSignal(sound *, char *);
int writeFileRaw(sound *, char *);
int createWindow(feS *, float *);
int computeDeriv(feS *, float *, float *, float *);
int loadConfiguration(feS *, bf ***, hrw *,const char *);
void *readBuffer(void **);
void *processBuffer(void **);
int readFileRaw(feS *, char *, sound *);
int readFileWAV(feS *, char *, sound *);
int readFileAD(feS *, char *, sound *);

float melScale(float);
float melScaleInv(float);

void decode_online(search_t *search, acquisitor_t *acquisitor, lattice_t *lattice);

#endif
