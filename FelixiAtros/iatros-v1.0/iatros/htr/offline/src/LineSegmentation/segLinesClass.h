/* g++ -Wall -Wno-sign-compare -o pgmSegLines pgmSegLines.cc pgmClass.cc */
#ifndef SEGLINESCLASS_H
#define SEGLINESCLASS_H

#include <math.h>
#include <fftw3.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "../data_sets/pgmClass.h"
#include "../data_sets/mfSetClass.h"

#define LONGITUD_MEDIA_LETRA 40  // Para RLSA
#define SMOOTH_FACTOR        10  // Factor de suavizado de histHoriz
#define PORC_MIN_LINE_HEIGHT 60  // Porcentaje del alto de línea considerado
#define FFT_SEARCH_FROM      10  // Comienza la búsqueda en el espectro desde


using namespace std;


class datcomp {
 public:
  unsigned int st, fn, pc, ci, cf;
  datcomp (): st(0), fn(0), pc(0), ci(0), cf(0) {}
  datcomp (int _a, int _b): st(_a), fn(_b), pc(0), ci(0), cf(0) {}
  datcomp (int _a, int _b, int _c, int _d, int _e): st(_a), fn(_b), pc(_c), ci(_d), cf(_e) {}
  datcomp & operator =(const datcomp & p) { st=p.st; fn=p.fn; pc=p.pc; ci=p.ci; cf=p.cf; return *this; }
  bool operator <(const datcomp & p) const { return st<p.st; }
  //bool operator ==(datcomp & p) const { return st==p.st; }
  //bool operator >(datcomp & p) const { return st>p.st; }
};


class lineSegm: public pgmimage {
  float * histHoriz;
  float med, desv;
  vector<datcomp> clusterlines;
  vector<datcomp> lines;
  // private methods
  void printImageSegValleys();
  void printImageSegPics();
  void printImageSeg();
  void makeHistHorizNorm(int thresh, int margin=0);
  void smoothHistHoriz(int smth_widht=2);
  void makeHistVertNorm(int srow, int erow, int thresh);
  bool isShortLine(int srow, int erow, int thresh);
  void searchTextLines(int min_height, float min_length);
  //void searchPics(float min_base_height=50, int min_space=20);
  void searchValleysAndPics(float min_base_height=50, int min_space=20);
  void searchValleysAndPicsForShortLines(float line_height, int thresh);
  void writeLineSegmOverlap(char *fname, unsigned overlap);
  void writeLineSegmDetAsc(char *fname, unsigned thresh);
  int otsu();
  float* makeHistGreyLevelNorm();
  void mean_double(float * h, int k, float & mu1, float & mu2, float & muT);
  void rlsaHoriz(int thresh);
  void rlsaLineVert(int srow, int erow, int thresh);
  void readSegmInf(char * ifname);
  void readSegmInfandImage(char * ifname,char * imgname);
  float determineLineHeightFFT(int searchFrom);
 public:
  // public methods
  lineSegm(); // constructor
  lineSegm(lineSegm & otra, unsigned irow, unsigned erow); // copy constructor
  ~lineSegm(); // destructor
  void writeSegmInf(char * fname);  
  void makeSegLines(char * fname, int overlap, int level=0, bool divline=false, bool demo=false);
  void fileSegLines(char * ifname, char * sfname,char * imgname, int overlap, bool demo=false);
};

#endif
