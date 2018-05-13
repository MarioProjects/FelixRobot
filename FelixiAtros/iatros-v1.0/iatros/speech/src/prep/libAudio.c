/** @defgroup prep The preprocess module
 *  This module contains all the functions for audio preprocessing
 *  @{
 */

/** @page authors Authors
 *  Vicent Tamarit - vtamarit@iti.upv.es
 *  February 2008
 */


#include <iatros-speech/config.h>
#include <prep/libAudio.h>
#include <prep/circularBuffer/circularBuffer.h>
#include <prhlt/constants.h>
#include <alloca.h>



// redefine the macro to remove the assert(ptr). See DEVEL.NOTES
#undef snd_pcm_hw_params_alloca
#define snd_pcm_hw_params_alloca(ptr) do { *ptr = (snd_pcm_hw_params_t *) alloca(snd_pcm_hw_params_sizeof()); memset(*ptr, 0, snd_pcm_hw_params_sizeof()); } while (0)

pthread_mutex_t mutexBuffer=PTHREAD_MUTEX_INITIALIZER; ///< This mutex coordinates the use of the buffer between the record funcion and the parametrization one


pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER; ///< Used by te conditional variables
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER; ///< Used by te conditional variables


/**
   This function acts like an iterator over the signal.
   Each call writes the next buffer into the structure
   @param[in] FE Structure with feature extraction parameters
   @param[in] input Sound structure
   @return 0 if there is no problem.
   @return -1 Indicates that there are no more buffers.
*/
int nextBuffer(feS *FE, sound *input){

  int i;

  if(input->buffer==NULL){ //There is no vector
    input->buffer=(short int *)malloc(sizeof(short int)*FE->iFrameSize);
  }

  for(i=0;i<FE->iFrameSize;i++){

    //Complete the buffer with 0s
    if((i+input->lastSample) >= input->iSamples){
      input->buffer[i]=0;
    }
    else{
      input->buffer[i]=input->data[i+input->lastSample];
    }
  }

  input->lastSample+=FE->iFrameShift;

  if(input->lastSample > input->iSamples){
    //No more buffers
    return -1;
  }

  return 0;

}

/**
   Initialize the FE structure
    @return pointer to the FE structure
*/
feS *createFE(){

  feS *FE=(feS *)malloc(sizeof(feS));

  FE->iSampleRate=0;
  FE->iSubSampleRate=0;
  FE->iFFTsize=0; 
  FE->iFrameSize=0;
  FE->fWindowLen=0;
  FE->iFrameShift=0;
  FE->nFilters=23;
  FE->bTriangular=1;
  FE->fFactor=0.97;
  FE->fWindowAlfa=0.54; 
  FE->iCCs=13; 
  FE->fSecondsSilence=0; 
  FE->fSilenceThreshold=0; 
  FE->iSizeSignalBuffer=0; 
  FE->iSizeCCBuffer=0; 
  FE->fCosTable=NULL; 
  FE->iDerivative=2 ; 

  return FE;

}

/**
   Initialize the sound structure
    @return pointer to the sound structure
*/
sound *createSound(){

  sound *output;

  output=(sound *)malloc(sizeof(sound));

  output->iBytes=0;
  output->audio=NULL;
  output->data=NULL;
  output->iSamples=0;
  output->fTime=0;
  output->lastSample=0;
  output->buffer=NULL;
  output->lastSample=0;
  output->prior=0;

  return output;

}

/**
   Clear the sound structure
   @param[in] output Sound structure
   @return 0 if ther is no problem
*/
int clearSound(sound *output){

  output->iBytes=0;
  output->audio=NULL;//(char *)realloc(output->audio,0);
  output->data=NULL;//(short int *)realloc(output->data,0);
  output->iSamples=0;
  output->fTime=0;
  output->lastSample=0;
  output->buffer=NULL;//(short int *)realloc(output->buffer,0);
  output->lastSample=0;
  output->prior=0;

  return 0;

}

/**
   Calculate FFT of the buffer(with hamming window)
   @param[in] FE Structure with feature extraction parameters
   @param[in] in Input signal
   @param[out] out Output vector with FFT. Size: len(in)/2+1
   @return 0 if there is no problem
*/
int calculateFFT(feS *FE, float *in, float *out){

  int i;
  double *input;
  fftw_complex *output;
  fftw_plan p;

  input = (double*) fftw_malloc(sizeof(double) * FE->iFFTsize);
  output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(int)((FE->iFFTsize/2)+1));

  p=fftw_plan_dft_r2c_1d(FE->iFFTsize, input, output, FFTW_MEASURE);

  for(i=0;i<FE->iFrameSize;i++){
     input[i]=(double)in[i];
  }

  for(i=FE->iFrameSize;i<FE->iFFTsize;i++){
     input[i]=(double)0.0;
  }

  fftw_execute(p);
  fftw_destroy_plan(p);


  for(i=0;i<FE->iFFTsize/2+1;i++){

    out[i]=output[i][0]*output[i][0]+output[i][1]*output[i][1];

  }

  fftw_free(input);
  fftw_free(output);

  return 0;
}

/// Aply the filters to the buffer
/**
   @param[in] FE Structure with feature extraction parameters
   @param[in] in Input vector (comes from FFT)
   @param[out] out Output vector
   @param[in] filter Pointer to the vector of filters
   @return 0 if there is no problem
*/
int applyFilterBank(feS *FE, float *in, float *out, bf **filter){

  int k,k1,k2;
  int i, indexNow=0;


  //For each filter...
  for(i=0;i<FE->nFilters;i++){

    k1=indexNow;
    k2=indexNow+filter[i]->index-1;

    for(k=k1+1;k<k2;k++){
      out[i]+=in[k];
    }

    /* First point: */
    if (i == 1) {
      out[i] += in[k1];
    } else {
      out[i] += in[k1] / 2;
    }
    /* Last point */
    out[i] += in[k2] / 2;

    out[i]=out[i]/filter[i]->index;

    if (out[i] > 0) {
      out[i] = (float)(log( (double)out[i]) * 1000);

    } else {
      out[i] = -1.0e+5;
    }

    indexNow+=filter[i]->index-1;

  }

  return 0;
}

float melScale(float in){

  return 2595*log10(1+(in/700));

}

float melScaleInv(float in){

  return (pow(10,(in/2595)) - 1 )*700 ;

}

/// Computes and apply the triangular filters to the buffer
/**
   @param[in] FE Structure with feature extraction parameters
   @param[in] in Input vector (comes from FFT)
   @param[out] out (output vector)
   @return 0 if there is no problem
*/
int applyTriangularFilterBank(feS *FE, float *in, float *out){

  int i, k;
  float fStart=64;
  float fc[FE->nFilters+1];
  int cbin[FE->nFilters+2];

  float sum1, sum2;
  
  cbin[0]=round((fStart/FE->iSampleRate)*FE->iFFTsize);
  cbin[24]=FE->iFFTsize/2;

  for(i=1;i<=FE->nFilters;i++){
    
    fc[i]=melScaleInv(melScale(fStart) + 
		      ((melScale(FE->iSampleRate/2)-melScale(fStart))*(i)) /
		      (FE->nFilters+1)
		      );

    cbin[i]=round((fc[i]/FE->iSampleRate)*FE->iFFTsize);
    //    fprintf(stderr, "Prueba filtros: %d\n", cbin[i]);

  }

  for(k=1;k<=FE->nFilters;k++){
     
    sum1=0;
    for(i=cbin[k-1];i<=cbin[k];i++){
      sum1+=((i-cbin[k-1]+1)/(cbin[k]-cbin[k-1]+1))*in[i];
    }

    sum2=0;
    for(i=cbin[k]+1;i<=cbin[k+1];i++){
      sum2+= ( 1-( (i-cbin[k]) / (cbin[k+1]-cbin[k] +1) ) )*in[i];
    }
     
    out[k-1]=sum1+sum2;

    if (out[k-1] > 2e-22) {
      out[k-1] = (float)(log( (double)out[k-1]) *1000 );
      
    } else {
      out[k-1] = 0;
    }
  }

  return 0;
}


/// It does the cosine transformation, the last step in CC extraction.
/**
   @param[in] FE Structure with feature extraction parameters
   @param[in] in Input vector (comes from applyFilterBank)
   @param[out] out Output vector (the cepstrals)
   @return 0 if there is no problem
*/
int extractCC(feS *FE, float *in, float *out){

  int i,j;


  for(i=0;i<FE->iCCs;i++){
    out[i]=0.0;
    for(j=0;j<FE->nFilters;j++){
 	out[i]+=(in[j]*FE->fCosTable[i][j]);

    }
    out[i]=out[i]/(FE->nFilters);

  }

  return 0;
}

/// Calculates the preemphasis of the signal
/**
   @param[in] FE Structure with feature extraction parameters
   @param[in] in Input signal
   @param[out] out Output Signal
   @param[in] prior The last value of the previous buffer
   @return 0 if there is no problem
*/

int preemphasis(feS *FE, short int *in, float * out, int prior){

  int i;

  if(FE->fFactor!=0){
    out[0]=(float)(in[0]-((float)prior*FE->fFactor));
    for(i=1;i<FE->iFrameSize;i++){
      out[i]=(float)(in[i]-((float)in[i-1]*FE->fFactor));
    }

  }
  else{
    for(i=0;i<FE->iFrameSize;i++){
      out[i]=(float)(in[i]);
    }

  }

  return 0;
}

/// The Initialization function
/**
   Allocates memory for the atributes of the class
   @param handle The pointer to the sound device
   @param[in] hrwParam Structure with the hardaware parameters which the hardware will be initilized
   @param[out] params The parameters are sotored in this ALSA structure
   @return 0 if there is no problem
*/
int initHardware(snd_pcm_t *handle, hrw *hrwParam, snd_pcm_hw_params_t *params){
  int err;
  int dir;
  //snd_pcm_uframes_t frames;

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */
  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

  /* Set channels*/
  snd_pcm_hw_params_set_channels(handle, params, hrwParam->iChannels);

  /* Set sampling rate*/
  snd_pcm_hw_params_set_rate_near(handle, params, &(hrwParam->iSampleRate), &dir);

  /* Set period size*/
  snd_pcm_hw_params_set_period_size(handle, params, hrwParam->iFrames, dir);

  /* Write the parameters to the driver */
  if ((err=snd_pcm_hw_params(handle, params))< 0) {
    fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(err));
    exit(1);
  }

  return 0;

}


/// Play sound
/**
   Play sound in raw format
   @param[in] hrwParam Structure with the hardaware parameters which the hardware will be initilized
   @param[in] output Pointer to the sound in memory
   @return 0 if there is no problem
*/
int playSound(hrw *hrwParam, sound *output){
  //long loops;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params=NULL;
  int dir;
  snd_pcm_uframes_t frames;
  char *buffer;
  int err;
  char *i;
  //int iBytes=output->iBytes;

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Open PCM device for recording (capture). */
  if ((err= snd_pcm_open(&handle, hrwParam->outputDevice, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
      fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(err));
      exit(1);
    }

  //Init Hardware with the settings stored in hrwParam
  initHardware(handle, hrwParam, params);

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);

  size = frames * (hrwParam->iBits/8) * hrwParam->iChannels; /* 2 bytes/sample, 1 channel */
  buffer = (char *) malloc(size);

  char *p=output->audio;

  for(i=p;i<p+output->iBytes;i=i+size){

    //rc = snd_pcm_writei(handle, buffer, frames);
    err = snd_pcm_writei(handle, i, frames);
    if (err == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (err < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(err));
    }  else if (err != (int)frames) {
      fprintf(stderr,
              "short write, write %d frames\n", err);
    }
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  // As can be seen in the examples of ALSA this function is needed,
  // but its use may cause segmentation fault
  //snd_pcm_hw_params_free(params);
  free(buffer);


  return 0;
}

/// Read sound from RAW file
/**
   Reads data from file
   @param[in] FE Structure with feature extraction parameters
   @param[in] nameFile Name of the raw file
   @param[out] input Pointer to the new sound in memory
   @return 0 if there is no problem
*/
int readFileRaw(feS *FE, char *nameFile, sound *input){

  int fileIn;

  if(nameFile!=NULL)
    fileIn=open(nameFile,O_RDONLY);
  else
    fileIn=0;

  input->iBytes=lseek(fileIn,0,SEEK_END);
  lseek(fileIn,0,SEEK_SET);

  input->audio=(char *)malloc(input->iBytes);

  if(read(fileIn,input->audio,input->iBytes)<0){
    error(1,errno,"An error ocurred reading %s",nameFile);

  }

  input->data=(short int *)input->audio;
  input->iSamples=input->iBytes/2;
  input->fTime=(float)input->iSamples/FE->iSampleRate;
  input->lastSample=0;


  return 0;
}

/// Read sound from AD file
/**
   Reads data from file
   @param[in] FE Structure with feature extraction parameters
   @param[in] nameFile Name of the raw file
   @param[out] input Pointer to the new sound in memory
   @return 0 if there is no problem
*/
int readFileAD(feS *FE, char *nameFile, sound *input){

  int fileIn;

  if(nameFile!=NULL)
    fileIn=open(nameFile,O_RDONLY);
  else
    fileIn=0;

  input->iBytes=lseek(fileIn,0,SEEK_END)-1024;
  //Avoid the first 1024 bytes
  lseek(fileIn,1024,SEEK_SET);

  input->audio=(char *)malloc(input->iBytes);

  if(read(fileIn,input->audio,input->iBytes)<0){

    error(1,errno,"An error ocurred reading %s",nameFile);

  }

  input->data=(short int *)input->audio;
  input->iSamples=input->iBytes/2;
  input->fTime=(float)input->iSamples/FE->iSampleRate;
  input->lastSample=0;
  return 0;
}

/// Read sound from WAV file
/**
   Reads data from file
   @param[in] FE Structure with feature extraction parameters
   @param[in] nameFile Name of the raw file
   @param[out] input Pointer to the new sound in memory
   @return 0 if there is no problem
*/
int readFileWAV(feS *FE, char *nameFile, sound *input){

  int fileIn;
  short int format;

  if(nameFile!=NULL)
    fileIn=open(nameFile,O_RDONLY);
  else
    fileIn=0;

  //Jump the first chunk
  lseek(fileIn,12,SEEK_CUR);

  //Second chunk
  lseek(fileIn,8,SEEK_CUR);

  read(fileIn,&format,2);

  if(format!=1){
    error(1,errno,"The file %s has some form of compression",nameFile);
  }

  lseek(fileIn,12,SEEK_CUR);

  //Third chunk

  lseek(fileIn,4,SEEK_CUR);

  read(fileIn,&input->iBytes,4);

  input->audio=(char *)malloc(input->iBytes);

  if(read(fileIn,input->audio,input->iBytes)<0){
    error(1,errno,"An error ocurred reading %s",nameFile);
  }

  input->data=(short int *)input->audio;
  input->iSamples=input->iBytes/2;
  input->fTime=(float)input->iSamples/FE->iSampleRate;
  input->lastSample=0;
  return 0;

}

/**
   Calculate the hamming window
   @param[in] FE Structure with feature extraction parameters
   @param[in] in Sound buffer
   @param[out] out Sound buffer (output)
   @param[in] window Window that will be applied to the buffer
   @return 0 if there is no problem
*/
int hammingWindow(feS *FE, float *in, float *out, float *window){

  int i;

  for(i=0;i<FE->iFrameSize;i++){
    out[i]=(float)window[i]*(float)in[i];

  }

  return 0;
}

/**
   Writes signal values to a file
   @param[in] input Pointer to the sound in memory
   @param[in] nameFile Name of the raw file
   @return 0 if there is no problem
*/
int printSignal(sound *input, char *nameFile){

  int i;
  FILE *fileOut;

  fileOut=fopen(nameFile,"w");

  for(i=0;i<input->iSamples;i++){
     fprintf(fileOut,"%d\n",input->data[i]);
  }

  fclose(fileOut);

  return 0;
}


/// Write sound to file
/**
   Writes data to file
   @param[in] nameFile Name of the raw file
   @param[in] output Pointer to the sound in memory
   @return 0 if there is no problem
*/
int writeFileRaw(sound *output, char *nameFile){

  int fileOut;

  if (nameFile!=NULL) {
    mode_t mode = (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    fileOut=open(nameFile, O_WRONLY | O_CREAT | O_TRUNC, mode);
  }
  else
    fileOut=1; //stdout
  if(write(fileOut, output->audio, output->iBytes)<0){

    error(1,errno,"An error ocurred writting %s",nameFile);

  }

  return 0;
}

/// Creates the hammingWindow
/**
   @param[in] FE Structure with feature extraction parameters
   @param[out] window HammingWindow
   @return 0 if there is no problem
*/
int createWindow(feS *FE, float *window){

  int i;
  float twopi=2*M_PI;

  for(i=0;i<FE->iFrameSize;i++){
    window[i]=(float)(FE->fWindowAlfa-((1-FE->fWindowAlfa)*cos((twopi*(i))/ (FE->iFrameSize-1) )));
  }

  return 0;
}


/// Reads the configuration from a text file
/**
   @param[out] FE Structure where the information will be stored
   @param[out] filter Structure where the filters will be stored
   @param[out] hrwParam Structure where the hardware information will be stored
   @param[in] fileConfName Name of the configuration file
   @return Pointer to the feS structure
*/
int loadConfiguration(feS *FE, bf ***filter, hrw *hrwParam ,const char *fileConfName){

  //feS *output;
  FILE *confFile;
  char line[100];
  char one[20], two[20], three[20],  four[20];
  unsigned short int bFilter=0;
  unsigned short int bParameters=0;
  unsigned short int bDevice=0;
  unsigned short int bBuffers=0;

  int nF=0, i, j;

  confFile=fopen(fileConfName, "rb");

  FE->nFilters=-1;

  while ( !feof(confFile) )
    {
      fgets(line, 100, confFile);

      if(line[0]=='#')
	continue;

      if(!strcmp(line, "<filter>\n")){
	bFilter=1;

	fscanf(confFile,"%s %s\n", one, two);

	if(!strcmp(one, "TriangularFilters")){
	  FE->nFilters=atoi(two);
	  FE->bTriangular=1;
	}
	else{
	  if(!strcmp(one, "FilterBankSize")){
	    FE->bTriangular=0;    
	    FE->nFilters=atoi(two);
	    (*filter)=(bf **)malloc(sizeof(bf *)*FE->nFilters);
	    for(i=0;i<FE->nFilters; i++)
	      (*filter)[i]=(bf *)malloc(sizeof(bf));
	  }
	  else
	    error(1,errno,"An error ocurred reading %s. The number of filter must be defined AFTER the filter label.",fileConfName);
	}
	continue;
      }
      if(!strcmp(line, "</filter>\n")){
	bFilter=0;
	continue;
      }

      if(!strcmp(line, "<parameters>\n")){
	bParameters=1;
	continue;
      }
      if(!strcmp(line, "</parameters>\n")){
	bParameters=0;
	continue;
      }

      if(!strcmp(line, "<device>\n")){
	bDevice=1;
	continue;
      }
      if(!strcmp(line, "</device>\n")){
	bDevice=0;
	continue;
      }

      if(!strcmp(line, "<buffers>\n")){
	bBuffers=1;
	continue;
      }
      if(!strcmp(line, "</buffers>\n")){
	bBuffers=0;
	continue;
      }

      if(bFilter){

	sscanf(line,"%s %s %s %s\n", one, two, three, four);

	(*filter)[nF]->fFrecStart=atof(two);
	(*filter)[nF]->fFrecEnd=atof(three);
	(*filter)[nF]->index=atof(four);

	nF++;
	continue;
      }

      if(bDevice){
	sscanf(line,"%s %s\n", one, two);
	if(!strcmp(one, "InputDevice")){
	  hrwParam->inputDevice=(char *)malloc(10*sizeof(char));
	  strncpy(hrwParam->inputDevice,two,8);

	}
	if(!strcmp(one, "OutputDevice")){
	  hrwParam->outputDevice=(char *)malloc(10*sizeof(char));
	  strncpy(hrwParam->outputDevice,two,8);
	}
	continue;
      }

       if(bBuffers){
	 sscanf(line,"%s %s\n", one, two);
	 if(!strcmp(one, "SizeSignal")){
	   FE->iSizeSignalBuffer=atoi(two);
	 }
	 if(!strcmp(one, "SizeCC")){
	   FE->iSizeCCBuffer=atoi(two);
	 }
       }

       if(bParameters){
	sscanf(line,"%s %s\n", one, two);
	if(!strcmp(one, "SampleFreq")){
	  FE->iSampleRate=atoi(two);
	  hrwParam->iSampleRate=atoi(two);
	}
	if(!strcmp(one, "SubSampleFreq")){
	  FE->iSubSampleRate=atoi(two);
	}
	if(!strcmp(one, "FFTlength")){
	  FE->iFFTsize=atoi(two);
	}
	if(!strcmp(one, "FrameSize")){
	  FE->iFrameSize=atoi(two);
	}
	if(!strcmp(one, "WindowLen")){
	  FE->fWindowLen=atof(two);
	}
	if(!strcmp(one, "FrameShift")){
	  FE->iFrameShift=atoi(two);
	}
	if(!strcmp(one, "Factor")){
	  FE->fFactor=atof(two);
	}
	if(!strcmp(one, "WindowAlfa")){
	  FE->fWindowAlfa=atof(two);
	}
	if(!strcmp(one, "CepCoefNumb")){
	  FE->iCCs=atoi(two);
	}
	if(!strcmp(one, "Channels")){
	  hrwParam->iChannels=atoi(two);
	}
	if(!strcmp(one, "Bits")){
	  hrwParam->iBits=atoi(two);
	}
	if(!strcmp(one, "Frames")){
	  hrwParam->iFrames=atoi(two);
	}
	if(!strcmp(one, "SecondsSilence")){
	  FE->fSecondsSilence=atof(two);
	}
	if(!strcmp(one, "SilenceThreshold")){
	  FE->fSilenceThreshold=atof(two);
	}
	if(!strcmp(one, "Derivative")){
	  FE->iDerivative=atoi(two);
	  if (FE->iDerivative>2 || FE->iDerivative<0){
	    error(1,errno,"The Derivative parameters must be between 0 and 2\n");
	  }
	}

	continue;
      }

      //fprintf(stderr,"%s\n",line);
    }

  //Some warnings
  if(FE->iFrameShift!=FE->iSampleRate/FE->iSubSampleRate){
    fprintf(stderr,"WARNING!: The Frame Shift parameter is bad calculated\n");
    fprintf(stderr,"FrameShift: %d - SampleRate: %d - Sub Sample: %d\n",FE->iFrameShift,FE->iSampleRate, FE->iSubSampleRate);

  }
  if(FE->iFFTsize<FE->iFrameSize){
    error(1,errno,"The FFT size must be bigger than frame size\n");
  }

  /*Calulate cosinus table*/

  FE->fCosTable=(float **)malloc(sizeof(float *)*FE->iCCs);
  for(i=0;i<FE->iCCs;i++){
    FE->fCosTable[i]=(float *)malloc(sizeof(float *)*FE->nFilters);
  }

  //This table is calculated only one time for each execution of the program
  for(i=0;i<FE->iCCs;i++){
    for(j=0;j<FE->nFilters;j++){
      //FE->fCosTable[i][j]=cos( ((M_PI*(i))/(2*(FE->iCCs))) * (2*j+1));
      FE->fCosTable[i][j]=cos( ( (M_PI * i * (j-0.5))/(FE->nFilters) ) ) ;
    }
  }
  fclose(confFile);

  return 0;
}

/// Read audio from buffer (online/offline)
/**
   This function is used by the on-line recogniser and the adquisition program.
   In the first case the function works with the buffers and activates the necessary thread.
   In the second case it only record the audio in a sound structure and returns it.

   @param[in] argTS Parameters of the function. This void double pointer is needed for the thread call
   @param[in] hrwParam (argTS[0]) Parameters of the hardware
   @param[in] circularBuffer (argTS[1]) Circular buffer for comunicate the sound record and the parametrization
   @param[in] record (argTS[2]) Sound estructure where the signal will be stored (useful for offline recognition)
   @param[in] iUse (argTS[3]) 0 indicates online use 1 indicates offline use (record)
   @param[in] FE (argTS[4]) Feature structure
   @return 0 if there is no problem
*/
void *readBuffer(void **argTS){//hrw *hrwParam, specialBuffer *circularBuffer){

  hrw *hrwParam=(hrw *)argTS[0];
  specialBuffer *circularBuffer=(specialBuffer *)argTS[1];
  sound *output=(sound *)argTS[2];
  int iUse=*((int *)argTS[3]);
  feS *FE=(feS *)argTS[4];

  long loops;
  float mean=0;

  int err;
  int sizeBytes;
  short int *sBuffer;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int uiTimePerFrame;
  int dir;
  snd_pcm_uframes_t frames;

  int i;
  char *buffer;

   /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);
  /* Open PCM device for recording (capture). */
  if ((err= snd_pcm_open(&handle, hrwParam->inputDevice, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
      fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(err));
      exit(1);
    }
  //Init Hardware with the settings stored in hrwParam
  initHardware(handle, hrwParam, params);

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  //fprintf(stderr, "frames cogidos: %d\n",frames );

  sizeBytes = frames * (hrwParam->iBits/8) * hrwParam->iChannels; /* 2 bytes/sample, 1 channel */
  buffer = (char *) malloc(sizeBytes*2);

  /* We want to loop for 1 second */
      snd_pcm_hw_params_get_period_time(params, &uiTimePerFrame, &dir);

  /*******************************/
  /*  Calculate mean of Silence  */
  /*******************************/
  if(FE->fSilenceThreshold==0){

 //Get one second to calculate noise
    loops = 1000000 / uiTimePerFrame;
  int loopsAux=(int)loops;
    fprintf(stderr, "Calculating noise...\n");
    while (loops > 0) {
      loops--;
      err = snd_pcm_readi(handle, buffer, frames);
      if (err == -EPIPE) {
        /* EPIPE means overrun */
        fprintf(stderr, "overrun occurred\n");
        snd_pcm_prepare(handle);
      } else if (err < 0) {
        fprintf(stderr, "error from read: %s\n", snd_strerror(err));
      } else if (err != (int)frames) {
        //fprintf(stderr, "short read, read %d frames\n", err);
      }

      sBuffer=(short int *)buffer;

      for (i=0;i<(sizeBytes/2);i++){
        mean+=abs(sBuffer[i]);
      }
    }

    mean=mean/(loopsAux*(sizeBytes/2));
    fprintf(stderr, "Noise:%f\n", mean);
  }
  else{
    mean=FE->fSilenceThreshold;
  }
  /*******************************/
  /* Start listening and record  */
  /*******************************/

  int bNonStop=1;
  int bRecording=0;
  float fMeanBuffer=0;
  int iSilenceDuration=FE->fSecondsSilence*1000000; // Duration of the silence
  float iBufSilence=0;
  float fIterSilence=iSilenceDuration/uiTimePerFrame;

  fprintf(stderr,"You can talk now...\n");

  while(bNonStop){
    err = snd_pcm_readi(handle, buffer, frames);
    if (err == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (err < 0) {
      fprintf(stderr, "error from read: %s\n", snd_strerror(err));
    } else if (err != (int)frames) {
      fprintf(stderr, "short read, read %d frames\n", err);
    }

    //Transform the signal into short integers (16 bits)
    sBuffer=(short int *)buffer;

    for (i=0;i<(sizeBytes/2);i++){
        fMeanBuffer+=abs(sBuffer[i]);
    }

    fMeanBuffer=fMeanBuffer/(sizeBytes/2);

    /* We see the mean to decide if there is
     sound or silence in the buffer */

    if((fMeanBuffer > 6*mean) && !bRecording){
      fprintf(stderr,"Start Recording...\n");
      bRecording=1;

      if(iUse==0){ //online
	//The sound begins half a circularBuffer->numElements before
	pthread_mutex_lock(&mutexBuffer);
	circularBuffer->forRead=((circularBuffer->forWrite+(sizeBytes/2)) +
				 (circularBuffer->numElements/2))%(circularBuffer->numElements);
	circularBuffer->numBytes=(circularBuffer->numElements/2)-(sizeBytes/2);

	pthread_mutex_unlock(&mutexBuffer);
      }
    }

    if(bRecording && (fMeanBuffer <= mean) ){
      iBufSilence++;
    }
    else{
      iBufSilence=0;
    }

    /* When there is no sound in fIterSilence buffers
       stops the recording */
    if (iBufSilence>fIterSilence) {

      bNonStop=0;
      fprintf(stderr,"Stop Recording...\n");

      if(iUse==1){ //Offline
	//Cut the signal that has no sound
	output->iBytes-=(sizeBytes*(iBufSilence-1));
	output->audio=(char *)realloc(output->audio,(output->iBytes)*sizeof(char));
      }

      break;
    }

    if(bRecording){
      /* Save signal */
      //err = write(1, buffer, sizeBytes);

      output->audio=(char *)realloc(output->audio,(output->iBytes+sizeBytes)*sizeof(char));
      for (i=0;i<(sizeBytes);i++){
	output->audio[output->iBytes+i]=buffer[i];
      }

      output->iBytes+=sizeBytes;

      if(iUse==0){ //recog online

	pthread_mutex_lock(&mutexBuffer);
	writeBuffer(circularBuffer, sBuffer, (int)(sizeBytes/2));
	pthread_mutex_unlock(&mutexBuffer);

	pthread_mutex_lock( &condition_mutex );
	if(circularBuffer->numBytes>=(unsigned int)FE->iFrameSize){
	pthread_cond_signal( &condition_cond );
	}
	pthread_mutex_unlock( &condition_mutex );
      }


      /* Print signal */
      /*for (i=0;i<(sizeBytes/2)-1;i++){
	fprintf(stderr, "%d\n",sBuffer[i]);
	}*/

    }
    else{

      if(iUse==0){ //recog online

	pthread_mutex_lock(&mutexBuffer);
	writeBuffer(circularBuffer, sBuffer, (int)(sizeBytes/2));
	pthread_mutex_unlock(&mutexBuffer);

      }

    }

  }

  //Stop a PCM preserving pending frames.
      snd_pcm_drain(handle);
  //Close the device.
      snd_pcm_close(handle);
      //    snd_pcm_hw_params_free(params);
    free(buffer);

  /*Complete the sound with all the information*/
  output->data=(short int *)output->audio;
  output->iSamples=output->iBytes/2;
  output->fTime=(float)output->iSamples/hrwParam->iSampleRate;
  if(iUse==0){ //recog online
    circularBuffer->valueAux=FE->fSilenceThreshold;

    pthread_mutex_lock( &condition_mutex );
    pthread_cond_signal( &condition_cond );
    pthread_mutex_unlock( &condition_mutex );
  }
  fprintf(stderr,"End adquisition thread\n");

  return NULL;

}

/// Process the on-line buffer
/**
   This function works with Online recognition (each one in a diferent thread)
   @param[in] argTP Parameters of the function.
   @param[in] filter (argTP[0]) Filters
   @param[in] circularBuffer (argTP[1]) Circular buffer of audio
   @param[in] FE (argTP[2]) Feature structure
   @param[in] circularBufferCC (argTP[3]) Circular Buffer of ceptrals
   @param[in]aux (argTP[4]) Sound structure
   @param[in] mutexBufferVec (argTP[5]) Mutex for syncronize the buffer between threads
   @param[in] condition_online_mutex  (argTP[6]) Condition variable
   @param[in] condition_online_cond  (argTP[7]) Condition variable
   @return 0 if there is no problem
*/
void *processBuffer(void **argTP){

  bf **filter=(bf **)argTP[0];
  specialBuffer *circularBuffer=(specialBuffer *)argTP[1];
  feS *FE=(feS *)argTP[2];
  specialBufferVec *circularBufferCC=(specialBufferVec *)argTP[3];
  sound *aux=(sound *)argTP[4];
  pthread_mutex_t *mutexBufferVec=(pthread_mutex_t *)argTP[5];
  pthread_mutex_t *condition_online_mutex = (pthread_mutex_t *)argTP[6];
  pthread_cond_t  *condition_online_cond  = (pthread_cond_t  *)argTP[7];

  int i;

  short unsigned int nonStop=1;


  float *resultEmphasis;
  float *resultHamming;
  float *resultFFT;
  float *resultBF;
  float *resultCC;
  resultEmphasis=(float *)malloc(sizeof(float)* FE->iFrameSize);
  resultHamming=(float *)malloc(sizeof(float)* FE->iFrameSize);
  resultFFT=(float *)malloc(sizeof(float)* ((FE->iFFTsize/2)+1));
  resultBF=(float *)malloc(sizeof(float)* (FE->nFilters));
  resultCC=(float *)malloc(sizeof(float)* (FE->iCCs));

  float **Vec_CC=(float **)malloc(sizeof(float *) * BUF_DERIV);

  for(i=0;i<BUF_DERIV;i++){
    Vec_CC[i]=(float *)malloc(sizeof(float) * ((FE->iCCs)*3));
  }


  float **buffCC=(float **)malloc(sizeof(float *) * BUF_DERIV);

  for(i=0;i<BUF_DERIV;i++){
    buffCC[i]=(float *)malloc(sizeof(float) * ((FE->iCCs)*3));
    }

  int frameNow=-1;
  int numFrames=0;

  float *fHammingWindow;
  fHammingWindow=(float *)malloc(sizeof(float)* FE->iFrameSize);
  createWindow(FE,fHammingWindow );

  int t=0;
  while(nonStop){

    //Wait for enought data

    pthread_mutex_lock( &condition_mutex );
    if(circularBuffer->numBytes<(unsigned int)FE->iFrameSize && circularBuffer->valueAux==VALUE_AUX){
      pthread_cond_wait(&condition_cond,&condition_mutex);
       pthread_mutex_unlock( &condition_mutex );
    }
    else{
      pthread_mutex_unlock( &condition_mutex );
    }

    if(circularBuffer->valueAux!=VALUE_AUX && circularBuffer->numBytes<(unsigned int)FE->iFrameSize ){ //The last buffer

      pthread_mutex_lock(&mutexBuffer);
      readFromBufferLast(circularBuffer,aux->buffer, FE);
      pthread_mutex_unlock(&mutexBuffer);
      nonStop=0;
    }
    else{
      pthread_mutex_lock(&mutexBuffer);
      readFromBuffer(circularBuffer,aux->buffer, FE);
      pthread_mutex_unlock(&mutexBuffer);
    }
    t++;

    //Clear pre-emphasis buffer
    for(i=0;i<FE->iFrameSize;i++){
      resultEmphasis[i]=0.0;
    }
    preemphasis(FE, aux->buffer, resultEmphasis, aux->prior);
    aux->prior=aux->buffer[FE->iFrameSize-1];

    //Clear hamming buffer
    for(i=0;i<FE->iFrameSize;i++){
      resultHamming[i]=0.0;
    }

    hammingWindow(FE, resultEmphasis, resultHamming, fHammingWindow);


    //Clear FFT buffer
    for(i=0;i<FE->iFrameSize/2+1;i++){
      resultFFT[i]=0.0;
    }

    calculateFFT(FE, resultHamming, resultFFT);

    //Clear BF buffer
    for(i=0;i<FE->nFilters;i++){
      resultBF[i]=0.0;
    }

    if (FE->bTriangular){
      applyTriangularFilterBank(FE, resultFFT, resultBF);
    }
    else{
      applyFilterBank(FE, resultFFT, resultBF, filter);
    }

    //Clear CC buffer
    for(i=0;i<FE->iCCs;i++){
      resultCC[i]=0.0;
    }
    extractCC(FE, resultBF, resultCC);
    frameNow++;
    numFrames=DerivativeFrameHTK(resultCC, Vec_CC, buffCC, frameNow, FE->iCCs);

    //Copy the finished feature vector to the buffer and wake up the recog thread
    if ((numFrames >= 1) && (numFrames < BUF_DERIV)){
      for (i = 0; i < numFrames; i++){

	pthread_mutex_lock(mutexBufferVec);

	writeBufferVec(circularBufferCC, Vec_CC[i],(FE->iCCs)*3);
	pthread_mutex_unlock(mutexBufferVec);
      }
    }

    pthread_mutex_lock( condition_online_mutex );

    if(circularBufferCC->numCC>0){
      pthread_cond_signal( condition_online_cond );
    }
    pthread_mutex_unlock( condition_online_mutex );

  }//End while


  frameNow++;
  numFrames=DerivativeFrameHTK(NULL, Vec_CC, buffCC, frameNow,FE->iCCs);

  if ((numFrames >= 1) && (numFrames < BUF_DERIV)){
    for (i = 0; i < numFrames; i++){
      pthread_mutex_lock(mutexBufferVec);
      writeBufferVec(circularBufferCC, Vec_CC[i],(FE->iCCs)*3);
      pthread_mutex_unlock(mutexBufferVec);
    }
    circularBufferCC->valueAux=numFrames;
  }

  //Awake the recognition thread
  pthread_mutex_lock( condition_online_mutex );
  pthread_cond_signal( condition_online_cond );
  pthread_mutex_unlock( condition_online_mutex );



  free(resultEmphasis);
  free(resultHamming);
  free(resultFFT);
  free(resultBF);
  free(resultCC);
  free(fHammingWindow);
  free(buffCC);

  fprintf(stderr,"End process thread\n");
  return NULL;

}


/// Obtain a lattice from a microphone acquisition
/**
@param search Search status
@param acquisitor Microphone status ready to acquire
@param lattice Output lattice
*/
void decode_online(search_t *search, acquisitor_t *acquisitor, lattice_t *lattice) {

  float *feat_vec = (float *)malloc(sizeof(float) * acquisitor_num_features(acquisitor));

  acquisitor_start(acquisitor);
  //fprintf(stderr,"no inicio\n");

  if (acquisitor_has_next(acquisitor)) {
    acquisitor_get_next(acquisitor, feat_vec);

    initial_stage(search, feat_vec, lattice);

    while (acquisitor_has_next(acquisitor)) {
      acquisitor_get_next(acquisitor, feat_vec);

      viterbi_frm(search, feat_vec, lattice);
    }

    end_stage(search, lattice);

    free(feat_vec);

    if (ENABLE_STATISTICS >= SV_SHOW_SAMPLE) {
      fprintf(stderr, "sample stats:\n");
      print_sample_stats(stderr, search->stats, search->n_frames);
      fprintf(stderr, "\n");
    }
  }
}


/** @} */ // end of prep
