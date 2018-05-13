#include "prep/libAudio.h"
#include <getopt.h>

void help(){
  printf("Usage of record program (iAtros)\n");
  printf("-h\t This help\n");
  printf("-c\t Configuration file (needed) \n");
  printf("-t\t Type of the sginal\n");
  printf("\t\t raw Raw format (default - only supported at this time)\n");
  printf("\t\t wav Wave format \n");
  printf("\t\t AD AD format \n");
  printf("-o\t Output File (default stdout) \n");
  printf("-p\t If this file is specified the signal will be stored as numbers in a plain text file (useful for plot the signal) \n");

}

int main(int argc, char *argv[]){


  int option;
  char *sConfigurationFile=NULL;
  char *sInputFile=NULL;
  char *sOutputFile=NULL;
  char *sPrintFile=NULL;
  char typeFile='r';

  while ((option=getopt(argc,argv,"c:p:o:t:"))!=-1){
    switch (option){
    case 't':
      if(!strcmp((char *)optarg,"raw"))
	typeFile='r';
      if(!strcmp((char *)optarg, "AD"))
	typeFile='a'; 
      if(!strcmp((char *)optarg, "wav"))
	typeFile='w'; 
      break;
    case 'c':
      sConfigurationFile=(char *)malloc(sizeof(char)*strlen(optarg)+1);
      strcpy(sConfigurationFile,optarg);
      break;
    case 'p':
      sPrintFile=(char *)malloc(sizeof(char)*strlen(optarg)+1);
      strcpy(sPrintFile,optarg);
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
    fprintf(stderr,"There is no file configuration\n");
    help();
    exit(1);
  }

  hrw *hrwParam;
  hrwParam =(hrw *)malloc(sizeof(hrw));

  sound *aux;
  aux=createSound();

  bf **filter;
  feS FE;
  loadConfiguration(&FE, &filter, hrwParam, sConfigurationFile);
 
  int iUse=1;
  void *argTS[5];
  argTS[0]=(void *)hrwParam;
  argTS[1]=NULL;
  argTS[2]=(void *)aux;
  argTS[3]=(void *)&iUse;
  argTS[4]=(void *)&FE;
  readBuffer(argTS);

  if(sPrintFile!=NULL)
    printSignal(aux, sPrintFile);

  writeFileRaw(aux, sOutputFile);
  
  playSound(hrwParam, aux);

  free(aux);
  return 0;

  free(sInputFile);
  free(sOutputFile);
  free(sPrintFile);
  free(sConfigurationFile);

  free(hrwParam->inputDevice);
  free(hrwParam->outputDevice);
  free(hrwParam);

  return 0;
}
