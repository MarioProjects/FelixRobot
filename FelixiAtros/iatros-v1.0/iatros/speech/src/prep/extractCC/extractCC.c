/** @page authors Authors
 *  Vicent Tamarit - vtamarit@iti.upv.es
 *  February 2008  
 */

#include "prep/libAudio.h"
#include <getopt.h>

int writeATROSHeader(int iCCs, int iDerivative, int lines, FILE *fileOutputFile){

  int i;

  fprintf(fileOutputFile,"Name            prova.1030428185830.1070215194551\n");
  fprintf(fileOutputFile,"Date            Thu Mar 15 19:45:51 2007\n");
  fprintf(fileOutputFile,"CorpusName      unknown\n");
  fprintf(fileOutputFile,"OrtTrans        micro\n");
  fprintf(fileOutputFile,"CommandLine     atrosd\n");
  fprintf(fileOutputFile,"ConfigFile      /home/vtamarit/dihana/AtrosV6.00/models/dihana.cnf\n");
  fprintf(fileOutputFile,"DataType        CepstrumCoef\n");
  fprintf(fileOutputFile,"Structure       Energy ");
  for(i=0;i<((iCCs)*(iDerivative+1))-1;i++)
    fprintf(fileOutputFile,"CC%d ",i);
  fprintf(fileOutputFile,"\n");
  fprintf(fileOutputFile,"NumParam        %d\n", (iCCs)*(iDerivative+1));
  fprintf(fileOutputFile,"NumVect         %d\n",lines);
  fprintf(fileOutputFile,"NumSatCC        0\n");
  fprintf(fileOutputFile,"Energy/CC       7.25\n");
  fprintf(fileOutputFile,"Data\n");

  return 0;
}

void help(){
  printf("Usage of offline preprocess program (iAtros)\n");
  printf("-h\t This help\n");
  printf("-c\t Configuration file (needed) \n");
  printf("-t\t Type of the sginal\n");
  printf("\t\t raw Raw format (default)\n");
  printf("\t\t wav Wave format \n");
  printf("\t\t AD AD format \n");
  printf("-i\t Input File \n");
  printf("-o\t Output File (default stdout) \n");

}
int main(int argc, char *argv[]){

  int option;
  char *sConfigurationFile=NULL;
  char *sInputFile=NULL;
  char *sOutputFile=NULL;
  char typeFile='r';
  FILE *fileOutputFile;

  while ((option=getopt(argc,argv,"c:i:o:t:"))!=-1){
    switch (option){
    case 't':
      if(!strcmp((char *)optarg,"raw"))
	typeFile='r';
      else
      if(!strcmp((char *)optarg, "AD"))
	typeFile='a'; 
      else
      if(!strcmp((char *)optarg, "wav"))
	typeFile='w';
      else{
	fprintf(stderr,"ERROR: The specified file type is not supported: %s\n\n", (char *)optarg);
	help();
	exit(1);
      }
      break;
    case 'c':
      sConfigurationFile=(char *)malloc(sizeof(char)*strlen(optarg)+1);
      strcpy(sConfigurationFile,optarg);
      break;
    case 'i':
      sInputFile=(char *)malloc(sizeof(char)*strlen(optarg)+1);
      strcpy(sInputFile,optarg);
      break;
    case 'o':
      sOutputFile=(char *)malloc(sizeof(char)*strlen(optarg)+1);
      strcpy(sOutputFile,optarg);
      break;
    case 'h':
      help();
      break;
    default:
      break;
    }
  }//End while

  if(sConfigurationFile==NULL){
    fprintf(stderr,"ERROR: There is no configurationfile !!\n\n");
    help();
    exit(1);
  }

  if(sOutputFile==NULL){
    fileOutputFile=stdout;
  }
  else{
    fileOutputFile=fopen(sOutputFile,"w");
  }

  
  if(sInputFile==NULL){
    fprintf(stderr,"ERROR: There is no input file !!\n\n");
    help();
    exit(1);
  }
  


  hrw *hrwParam;
  hrwParam =(hrw *)malloc(sizeof(hrw));

  int numVect=0;
  
  sound *aux;
  aux=createSound();

  /*Load Configuration from file*/

  bf **filter;
  feS *FE=createFE();
  //FE=(feS *)malloc(sizeof(feS));
  loadConfiguration(FE, &filter, hrwParam, sConfigurationFile);

  switch(typeFile){
  case 'r':
    readFileRaw(FE, sInputFile, aux);
    break;
  case 'a':
    readFileAD(FE, sInputFile, aux);
    break;
  case 'w':
    readFileWAV(FE, sInputFile, aux);
    break;
  }

    /* End Load configuration*/


  int has_next=1;
  int i,j;
  int sig=0;

  int points=0;
  int lines=0;

  //float sum; /*For estimation of the energy*/

  /*Binary File*/
  //char outfile[100];
  //strcpy(outfile, sInputFile);
  //strcat(outfile, ".mfc");

  
  //int fExit;
  //////////////fExit=open(outfile,O_WRONLY | O_CREAT | O_TRUNC );

  //fExit=fopen(outfile, "wb");
  
  lines=(((aux->iSamples)/
	   (FE->iSampleRate/FE->iSubSampleRate)));
  points=(lines*(FE->iCCs)*3); //Binary storage needs this value


  /************Salida Binaria***********/
  //////////////////write(fExit, &points, sizeof(int));

  writeATROSHeader(FE->iCCs, FE->iDerivative, lines, fileOutputFile);

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

  float lastCC[(FE->iCCs)*(FE->iDerivative+1)];
  
  for (i=0;i<(FE->iCCs)*(FE->iDerivative+1);i++){
    lastCC[i]=0;
  }

  while(has_next){
    sig=nextBuffer(FE, aux);
    if(sig==-1){
      has_next=0;
      break;
    }
    numVect++;
    
//Clear pre-emphasis buffer
    for(i=0;i<FE->iFrameSize;i++){
      resultEmphasis[i]=0.0;
    }
    preemphasis(FE, aux->buffer, resultEmphasis , aux->prior);
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


    //Clear filter buffer
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
    
    /*    //Estimate frame energy
    sum=0;
       for(i=0;i<FE->iFrameSize;i++){
      sum+=(aux->buffer[i])*(aux->buffer[i]);
      //           fprintf(stderr,"%f ",log(abs(aux->buffer[i])));
    }
    if (sum > 2e-22) {
      resultCC[0] = (float)(log( (double)sum) *1000 );
      
    } else {
      resultCC[0] = -50;
      }*/

    frameNow++;
    numFrames=DerivativeFrameHTK(resultCC, Vec_CC, buffCC, frameNow, (FE->iCCs));
    
    if ((numFrames >= 1) && (numFrames < BUF_DERIV)){
      for (i = 0; i < numFrames; i++){
	for(j=0;j<(FE->iCCs)*(FE->iDerivative+1);j++){
	  fprintf(fileOutputFile,"%f ",Vec_CC[i][j]);
	}
	fprintf(fileOutputFile,"\n");
      }
    }
    
    
  }//end while
  
  numFrames=DerivativeFrameHTK(NULL, Vec_CC, buffCC, frameNow,(FE->iCCs));
  
  if ((numFrames >= 1) && (numFrames < BUF_DERIV)){
    for (i = 0; i < numFrames; i++){
      for(j=0;j<(FE->iCCs)*(FE->iDerivative+1);j++){
	fprintf(fileOutputFile,"%f ",Vec_CC[i][j]);
      }
      fprintf(fileOutputFile,"\n");
    }
  }
  
  
  ////////////////////close(fExit);
  
  //fprintf(stdout,"%d, %d\n",ind, points);
  
  free(aux->buffer);
  free(aux->audio);
  free(aux);
  free(resultEmphasis);
  free(resultHamming);
  free(resultFFT);
  free(resultBF);
  free(resultCC);
  free(fHammingWindow);
  
  free(hrwParam->inputDevice);
  free(hrwParam->outputDevice);

  if (FE->bTriangular==0){ 
    for(i=0;i<FE->nFilters; i++)
      free(filter[i]);
    
    free(filter);
  }

  if(sOutputFile!=NULL){
    fclose(fileOutputFile);
  }
  
  free(sInputFile);
  free(sOutputFile);
  free(sConfigurationFile);
  free(hrwParam);


  
  return 0;
}
