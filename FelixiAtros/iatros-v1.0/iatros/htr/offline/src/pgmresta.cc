//------------------------------------------------------------
//    pgmresta.cc
//           aplica una mascara a una imagen.
//               Moisés Pastor i Gadea moises@iti.upv.es
//------------------------------------------------------------
// compilar: g++ -o pgmresta pgmresta.cc pgmClass.cc

#include <version.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include "data_sets/pgmClass.h"
#include "pgmresta.h"

using namespace std;

void restaClass::resta(const pgmimage & rlsaimage){
  for (int r=0;r<rows;r++)
    for (int c=0; c<cols; c++)
      if (rlsaimage.image[r][c]==maxval)
	      image[r][c]=maxval;
}

#ifndef PLUGIN

void version() {
    cerr << IATROS_HTR_OFFLINE_PROJECT_STRING"\n"IATROS_HTR_OFFLINE_BUILD_INFO;
}

void usage(char * nomProg){
  cerr << "Usage: "<<nomProg << " [options] -r rlsaImage" << endl;
  cerr << "      options:" << endl;
  cerr << "             -i inputfile  (by default stdin)" << endl;
  cerr << "             -o outputfile (by default stdout)" << endl;
  cerr << "             -v verbosityLevel (by default 0)" << endl;
  cerr << "             -V (version)"<< endl;
}

main (int argc, char ** argv){
  bool entrada_estandard=true,salida_estandard=true,resta_leida=false;
  int option;
  ifstream ifd,rfd;
  ofstream ofd;
  int verbosity=0;

  while ((option=getopt(argc,argv,"h:i:o:r:V"))!=-1)
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
    case 'r':
      rfd.open(optarg);
      if (!rfd){
	cerr << "Error: File \""<<optarg << "\" could not be open "<< endl;
	exit (-1);
      }
      resta_leida=true;
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

  if (!resta_leida){
    usage(argv[0]);
    exit(1);
  }

  restaClass image;
  if (entrada_estandard) {
    cerr << "entrada estandard\n";
    image.read(cin);
  } else
    image.read(ifd);
  
  pgmimage rlsaimage;
  rlsaimage.read(rfd);

  if (rlsaimage.rows != image.rows || rlsaimage.cols != image.cols){
    cerr << "Error: images with different size"<< endl;
    exit(-1);
  }
  image.verbosity=verbosity;
  image.resta(rlsaimage);

  if (salida_estandard) 
    image.write(cout,false);
  else
    image.write(ofd,false);
  
}

#endif

