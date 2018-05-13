#include "pgmClass.h"
#include <stdlib.h>

pgmimage::pgmimage(){
  image=0;
  rows=cols=0;
}
pgmimage::pgmimage (pgmimage & otra){
  rows=otra.rows;
  cols=otra.cols;
  maxval=otra.maxval;
  verbosity=otra.verbosity;

  image= new gray *[rows];
  for (int r=0; r<rows; r++)
    image[r]=new gray [cols];
  for (int r=0; r<rows;r++)
    for (int c=0;c<cols;c++)
      image[r][c]=otra.image[r][c];
}
pgmimage::pgmimage(int _r, int _c, gray maxv){
  rows=_r;
  cols=_c;
  maxval=maxv;

  image= new gray *[rows];
  for (int r=0; r<rows; r++)
    image[r]=new gray [cols];
  
  for (int r=0; r<rows;r++)
    for (int c=0;c<cols;c++)
      image[r][c]=maxval;
}
pgmimage::~pgmimage(){
  for (int r=0; r<rows; r++)
    delete [] image[r];
  delete [] image;
}
void  pgmimage::read(istream & fitx){
  char char1, char2,rubish[100000];
  string tmp;
  do{
  fitx >> char1;
  if(char1 == '#')
    fitx.getline(rubish,100000);
  } while (char1 == '#');
  fitx>> char2;
  //aceptamos solo ficheros pgm en ascii
  if (char1 != PGM_MAGIC1 || char2 != PGM_MAGIC2){ // || (char2 != PGRM_MAGIC2))){
    cerr << "Error: Bad magic number - not an ascii, pgm file" << endl;
    exit(-1);
  }
  
  //llegin columna i fila
  do{
  fitx >> tmp;
  if(tmp == "#")
    fitx.getline(rubish,100000);
  } while (tmp == "#");
  cols=atoi(tmp.c_str ());
  fitx >> rows;
  //llegim el max rang dels grisos
  int aux;
  fitx >> aux;
  maxval=gray(aux);
  if (maxval == 0){
    cerr << "Error: Maxval of input image is zero." << endl;
    exit(-1);
  }
  
  //demanem memoria
  image=new gray *[rows];
  for (int r=0; r<rows; r++)
    image[r]=new gray [cols];

  for (int r=0; r<rows;r++)
    for (int c=0;c<cols;c++){
      fitx >> aux;
      image[r][c]=gray(aux);
    }     
}
void  pgmimage:: write(ostream & fitx, bool binary, int irow, int erow){
  fitx << PGM_MAGIC1;
  if (binary)
    fitx << RPGM_MAGIC2 << endl;
  else 
    fitx << PGM_MAGIC2 << endl;
  
  if (!erow) erow=rows-1;
  fitx << cols << " " << erow-irow+1 << endl;
  fitx << int(maxval) << endl;
  for (int r=irow; r<=erow; r++){
    int charcount=1;
    for (int c=0; c<cols; c++){
      if ( charcount%65==0)
	fitx <<endl;
      fitx << int(image[r][c]) << " ";
    }
    fitx << endl;
    }
}
