//------------------------------------------------------------
//    pgmRLSA.cc
//          Enborronado de cada tramo original con el algoritmo de
//          suavizado por logitud de píxeles horizontales consecutivos 
//          "Run-Lengh Smoothing Algorithm" (RLSA).
//               Moisés Pastor i Gadea moises@iti.upv.es
//------------------------------------------------------------
// compilar: g++ -o pgmRLSA pgmRLSA.cc pgmClass.cc
//
#include <version.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include "data_sets/pgmClass.h"
#include "pgmRLSA.h"

float * rlsaClass::histo_norm() {
  float * hist=new float [maxval+1];
  
  for (int i=0;i<=maxval;i++)
    hist[i]=0;
  
  for (int r=0; r<rows; r++)
    for (int c=0; c<cols; c++){
      hist[image[r][c]]++;
    }
  
  int n_pixels=rows*cols;
  for (int i=0;i<=maxval;i++)
    hist[i]/=n_pixels;
  
  if (verbosity > 0){
    cerr << "# histograma "<< endl;
    for (int i=0; i<=maxval;i++)
      cerr << i<<"  " << hist[i] << endl;
  }

  return hist;
}
void rlsaClass::mean_double(float * h,int k, float & mu1, float & mu2, float & muT){
  float sum0=0, sum_tot0=0,sum1=0, sum_tot1=0;

  for (int i=0; i<=k; i++){
    sum0+=i*h[i];
    sum_tot0+=h[i];
  }
  mu1=(sum_tot0>0)?sum0/sum_tot0:0;
  //mu1=sum0/sum_tot0;
  for (int i=k+1; i<=maxval; i++){
    sum1+=i*h[i];
    sum_tot1+=h[i];
  }
  //mu2=sum1/sum_tot1;
  mu2=(sum_tot1>0)?sum1/sum_tot1:0;
  
  muT=(sum0+sum1)/(sum_tot0+sum_tot1);
}

int rlsaClass::otsu() {
  float * histo=histo_norm();
  float mu0,mu1,muT,max_sig_B2=0;
  int max_k=0;

  for (int k=0; k<= maxval;k++){
    mean_double(histo,k,mu0,mu1,muT);
    
    //prob a priori de la classe 0
    float Pr_C0=0;
    for (int i=0; i<=k; i++)
      Pr_C0+=histo[i];

    // funciÃ³ objectiu
    //float sig_B2=(Pr_C0)*(1-Pr_C0)*(mu0-mu1)*(mu0-mu1);
    float sig_B2=(Pr_C0)*(muT-mu0)*(muT-mu0)+(1-Pr_C0)*(muT-mu1)*(muT-mu1);
    // optimitzaciÃ³
    if ( sig_B2 > max_sig_B2){
      max_sig_B2 = sig_B2;
      max_k=k;
    }
  }
  delete [] histo;
  return max_k;
}

void rlsaClass::rlsa(const int longMediaLetra){
  int corte_histo=otsu();
  for (int row=0;row<rows;row++){
    int cont=0;
    int firstTime=1;
    for (int col=0;col<cols;col++)
      if (image[row][col]<=corte_histo){
	if(firstTime)firstTime=0;
	else
	  if (cont < longMediaLetra)
	    for (int c2=col-cont; c2 <= col; c2++)
	      image[row][c2]=0;
	cont=0;
      } else
	cont++;
  }

  //ahora para columnas
 for (int c=0;c<cols;c++){
    int cont=0;
    int firstTime=1;
    for (int r=0;r<rows;r++)
      if (image[r][c]<=corte_histo){
	if(firstTime)firstTime=0;
	else
	  if (cont < longMediaLetra)
	    for (int r2=r-cont; r2 <= r; r2++)
	      image[r2][c]=0;
	cont=0;
      } else
	cont++;
  }
  
}

#ifndef PLUGIN

void version() {
    cerr << IATROS_HTR_OFFLINE_PROJECT_STRING"\n"IATROS_HTR_OFFLINE_BUILD_INFO;
}

void usage(char * nomProg){
  cerr << "Usage: "<<nomProg << " options" << endl;
  cerr << "      options:" << endl;
  cerr << "             -i inputfile  (by default stdin)" << endl;
  cerr << "             -o outputfile (by default stdout)" << endl;
  cerr << "             -l longMedChar (by default 40)" << endl;
  cerr << "             -v verbosityLevel (by default 0)" << endl;
  cerr << "             -V (version)"<< endl;
}

main (int argc, char ** argv){
  bool entrada_estandard=true,salida_estandard=true;
  int option;
  ifstream ifd;
  ofstream ofd;
  int verbosity=0,longMediaLetra=40;

  while ((option=getopt(argc,argv,"h:i:o:l:V"))!=-1)
    switch (option)  {
    case 'i':
      ifd.open(optarg);
      if (!ifd){
	cerr << "Error: File \""<<optarg << "\" could not be open "<< endl;
	exit (-1);
      }
      entrada_estandard=false;
      break;
    case 'o':
      ofd.open(optarg);
      if (!ofd){
	cerr << "Error: File \""<<optarg << "\" could not be open "<< endl;
	exit (-1);
      }
      salida_estandard=false;
      break;
    case 'l':
      longMediaLetra=atoi(optarg);
      break;
    case 'v':
      verbosity=atoi(optarg);
      break;
    case 'V':
      version(); 
      break;
    case 'h':
    default:
      usage(argv[0]);
      exit(1);
    }


  rlsaClass image;
  if (entrada_estandard) {
    cerr << "entrada estandard\n";
    image.read(cin);
  } else
    image.read(ifd);

  image.verbosity=verbosity;
  image.rlsa(longMediaLetra);

  if (salida_estandard) 
    image.write(cout,false);
  else
    image.write(ofd,false);
  
}

#endif

