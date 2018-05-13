// compilar: g++ -o pbmclean pbmclean.cc mfSetClass.cc libpbm.c
#include <version.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include "pbmclean.h"
extern "C"{
#include "data_sets/libpbm.h"
}
#include "data_sets/mfSetClass.h"

using namespace std;

void version() {
    cerr << IATROS_HTR_OFFLINE_PROJECT_STRING"\n"IATROS_HTR_OFFLINE_BUILD_INFO;
}

void usage(char * progName,char * cad){
  fprintf(stderr,"Usage: %s %s\n",progName,cad);
  exit(1);
}

int compRepr(const void *a, const void* b){
  if (*(int *)a > *(int *)b)
    return 1;
  else if (*(int*)a < *(int*)b)
    return -1;
  else
    return 0;
}
#ifndef PLUGIN
int main(int argc, char *argv[]) {
 

  pbmclean_values vals;
  int option;  bit **res;
  vals.ifd=stdin;vals.ofd=stdout;
  vals.verbosity=0;
  vals.out_ascii=1;
  vals.pTall=100.0;

  vals.ifd=stdin;
  vals.ofd=stdout;
  char usagecad[]="[-i imput_image_file] [-o output_image_file] [-t threshold] [-h] [-d] [-b] [-v number] \n \
          Options are: \n \
                 -i imput_image_file      by default stdin \n \
                 -o output_image_file     by default stdout \n \
                 -t corte \n \
                 -h                       shows this information \n \
                 -v level                 verbosity mode \n \
                 -b                       binary output mode\n \
                 -V                       version\n";
  
  while ((option=getopt(argc,argv,"hbdt:v:i:o:V"))!=-1)
    switch (option)
      {
      case 'V':
  version();	
	break;
      case 'i':
	vals.ifd=fopen(optarg,"r");
	break;
      case 'o':
	vals.ofd=fopen(optarg,"w");
	break;
      case 'v':
	vals.verbosity=atoi(optarg);
	break;
      case 'b':
	vals.out_ascii=0;
	break;
      case 't':
	vals.pTall=atof(optarg);
	break;
      case 'h':
	usage(argv[0],usagecad);
	break;
      default:
	usage(argv[0],usagecad);
      }


  if (vals.ifd==NULL || vals.ofd==NULL) {
    fprintf(stderr,"El fichero de entrada o de salida no ha podido abrirse\n");
    exit(-1);
  } 
  if((res = pbmclean(&vals,NULL)) != NULL){
      pbm_freearray(res, vals.rows);
      return -1;
   } else return 0;


}

#endif

bit **pbmclean(pbmclean_values *vals, bit **input, bit black, bit white){
  bit **frase;
  int cols,rows,col,row;

  if(input ==NULL){
    frase=pbm_readpbm(vals->ifd, &vals->cols, &vals->rows);
    if(vals->ifd!=stdin)
      fclose(vals->ifd);
  }
  else{
    frase=input;
  }
  rows=vals->rows;
  cols=vals->cols;
  

  mfSet mf(cols*rows); // Definimos un mfset.

  for (row=0; row<rows-1; row++)      // Determinamos las componentes conexas.
    for (col=0;col<cols-1;col++) 
      if (frase[row][col]==black){
	int a  =row*cols+col;
	//int vN =a-cols;
	int vNE=a-cols +1;
	int vE =a+1;
	int vSE=vE+cols;
	int vS =a+cols;
	//int vSO=a+cols -1;
	//int vO =a-1;
	//int vNO=a-cols -1;
	
	//if (row > 0 && frase[row-1][col] == PBM_BLACK)
	//  mf.merge(a,vN);
	if (row > 0 && col < cols -1 &&  frase[row-1][col+1] == black)
	  mf.merge(a,vNE);
  if (col < cols -1 &&  frase[row][col+1] == black)
	  mf.merge(a,vE);
  if (row < rows-1 && col < cols -1 && frase[row+1][col+1] == black)
	  mf.merge(a,vSE);
  if (row < rows-1 && frase[row+1][col] == black)
	  mf.merge(a,vS);
	//if (row < rows -1 && col > 0 &&  frase[row+1][col-1] == PBM_BLACK)
	//  mf.merge(a,vSO);
	//if (col > 0 && frase[row][col-1] == PBM_BLACK)
	//  mf.merge(a,vO);
	//if (row > 0 && col > 0 && frase[row-1][col-1] == PBM_BLACK)
	//  mf.merge(a,vNO);
      }
    
  int nP = mf.Subconjuntos(); // Determinamos el n�mero de particiones y guarda-
  int  **representantes = (int **)malloc(2*sizeof(int*)); // mos en un vector los
  representantes[0] = (int *)malloc(nP*sizeof(int));
  representantes[1] = (int *)malloc(nP*sizeof(int));
  
  mf.obtenRepresentantes(representantes);      // representantes de cada clase.
 
 
  int sum=0,numR=0;
  int * rep=new int[nP];
  for (int i=0;i<nP;i++)
    if (representantes[1][i] < -1){
      sum+=representantes[1][i];
      rep[numR++]=representantes[1][i];
      if (vals->verbosity)
	cerr << representantes[1][i] << endl;
    }
 

  qsort(rep,numR,sizeof(int),compRepr);
  int acum=0,tall;
  for (int i=0;i<nP;i++)
    if (rep[i] < -1 && -acum < -(sum*vals->pTall)){
      acum+=rep[i];
      tall=rep[i];
    }
  
  //  cerr << "# sum= " << sum << " tall = "<< tall<< endl;
  
  for (row=0;row<rows;row++)
    for (col=0;col<cols;col++)
      if (mf.findCardinalitat(row*cols+col) > tall)
	frase[row][col]=white;
  if(vals->ofd!=NULL){  
    pbm_writepbm(vals->ofd,frase,cols,rows,1);
    if(vals->ofd!=stdout)
      fclose(vals->ofd);
  }

  return frase;
}
