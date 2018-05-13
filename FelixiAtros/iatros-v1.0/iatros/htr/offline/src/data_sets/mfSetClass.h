#ifndef MFSETCLASS_H
#define MFSETCLASS_H


#include "pgmClass.h"

using namespace std;

class mfSet {
  int *mfs;
  int n;
  int nSubconjuntos;
 public:
  mfSet(int talla); 
  ~mfSet(); 
  int find(int x);
  void merge(int x, int y); // Hace una unión por altura. La altura 
  // de x es -mfs[x]+1 y la de y es -mfs[y]+1. En caso de empate 
  // enlaza el nodo y al nodo x.
  int Subconjuntos();
  void obtenRepresentantes(int **v);
  int findCardinalitat(int x);
};

typedef struct {
  int ind_size;
  bool activate;
  int maxRow;
  int minRow;
} mfsData;

class MfSetPGM {
  mfsData *mfs;
  int n;
  int nSubSets;
  void init(int rows, int cols);
 public:
  int verbosity;
  MfSetPGM(int rows, int cols);
  MfSetPGM(gray **img, unsigned greyThrs, int irow, int erow, int cols);
  ~MfSetPGM(); 
  int findSet(int x);
  void merge(int x, int y); //  Create a x*y sets initialized each
                            //  of them with height=-1 and
                            //  maxRow=minRow=row of each
                            //  pixel. Height of list x is mfs[x].ind_size
                            //  and of list y is mfs[y].ind_size
  int numSubSets();
  void upperChosenSets(int *v, int &nE, int &maxRow, int ref, int iP, int eP);
  void lowerChosenSets(int *v, int &nE, int &minRow, int ref, int iP, int eP);
  int sizeSet(int x);
};

#endif
