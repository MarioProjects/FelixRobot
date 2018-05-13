//------------------------------------------------------------
//    pgmsubsract.cc
//           quita el fondo de la imagen original
//               Moises Pastor i Gadea moises@iti.upv.es
//------------------------------------------------------------
// compilar: g++ pgmsubstract.cc pgmClass.cc -o pgmsubstract
#include <version.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include "pgmsubstract.h"

using namespace std;

int compare_gray(const void *a, const void *b) {
  if (*(gray *)a < *(gray *)b) 
    return -1;
  else if (*(gray *)a > *(gray *)b)
    return 1;
  else 
    return 0;
}


pgmimage * pgmsubstract::median(int kernel) {
  gray *tmp;
  
  tmp = (gray*)malloc((kernel*kernel) * sizeof(gray));
  pgmimage * background=new pgmimage(rows,cols,255);
  kernel=kernel/2;
  
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      int n = 0;
      for (int i = row-kernel; i < row + kernel; i++)
        for (int j = col-kernel; j < col + kernel; j++){
	  int ni=(i<0)?0:i;
	  ni=(i>=rows)?rows-1:ni;
	  int nj=(j<0)?0:j;
	  nj=(j>=cols)?cols-1:nj;
	  
          tmp[n++] = image[ni][nj];

	}
      qsort(tmp, n, sizeof(gray), compare_gray);
      background->image[row][col] = tmp[n/2];
    }
  }
  
  if (verbosity)
    background->write(cerr,false);
  free(tmp);
  return background;
}

void pgmsubstract::substract(int k){
  pgmimage * background=median(k);
  for (int r=0; r<rows; r++)
    for (int c=0; c<cols; c++){
      int val=background->image[r][c] - image[r][c];
      val=(val<0)?0:val;    
      image[r][c]=maxval-val;
    }
}

void pgmsubstract::substract(pgmimage & subs){
  for (int r=0; r<rows; r++)
    for (int c=0; c<cols; c++){
      int val=subs.image[r][c] - image[r][c];
      val=(val<0)?0:val;    
      image[r][c]=maxval-val;
    }
}


//---------------------------------------------------
#ifndef PLUGIN

void version() {
    cerr << IATROS_HTR_OFFLINE_PROJECT_STRING"\n"IATROS_HTR_OFFLINE_BUILD_INFO;
}

void usage(char * nomProg){
  cerr << "Usage: "<<nomProg << " options " << endl;
  cerr << "      options:" << endl;
  cerr << "             -i inputfile  (by default stdin)" << endl;
  cerr << "             -o outputfile (by default stdout)" << endl;
  cerr << "             -s imageToSubstract" << endl;
  cerr << "             -k kernelSize (by default 3)"   << endl;
  cerr << "             -v verbosityLevel "<< endl;
  cerr << "             -V (version)"<< endl;
}

int main (int argc, char ** argv){
  bool entrada_estandard=true,salida_estandard=true,from_image=false;
  ifstream ifd,sfd;
  ofstream ofd;
  char option;
  int k=3,verbosity=0;
  
  while ((option=getopt(argc,argv,"hi:o:s:k:v:V"))!=-1)
    switch (option)  {
    case 'i':
      ifd.open(optarg);
      if (!ifd){
        cerr << "Error: File \"" << optarg << "\" could not be open "<< endl;
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
     case 's':
	sfd.open(optarg);
	if (!sfd){
	  cerr << "Error: File \""<<optarg << "\" could not be open "<< endl;
	  exit (-1);
	}
	from_image=true;
	break;
    case 'k':
      k=atoi(optarg);
      k=(k<3)?3:k;
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
    
  pgmsubstract image;

  image.verbosity=verbosity;

  if (entrada_estandard) {
    cerr << "entrada estandard\n";
    image.read(cin);
  } else
    image.read(ifd);
  
  if (from_image){
    pgmimage subs;
    subs.read(sfd);
    if(subs.rows != image.rows || subs.cols != image.cols){
      cerr << "Error: images with different size" <<endl;
      exit(-1);
    }
    image.substract(subs);
  } else
    image.substract(k);

  if (salida_estandard)
    image.write(cout,false);
  else if(ofd!=NULL)
    image.write(ofd,false);
  return 0;
}

#endif
