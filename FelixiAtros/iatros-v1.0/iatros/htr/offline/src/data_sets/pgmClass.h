#ifndef PGMCLASS_H
#define PGMCLASS_H

#include <iostream>

#define PGM_MAXVAL 255
/* Magic constants. */
#define PGM_MAGIC1 'P'
#define PGM_MAGIC2 '2'
#define RPGM_MAGIC2 '5'

using namespace std;

typedef unsigned char gray;

class pgmimage {
public:
  gray ** image;
  int rows, cols;
  gray maxval;
  int verbosity;
  pgmimage();
  pgmimage(int r, int c, gray maxv);
  pgmimage(pgmimage & otra);
  pgmimage(pgmimage & otra, unsigned irow, unsigned erow) {
    cerr << "ERROR: constructor 'pgmimage(pgmimage & otra, unsigned irow, unsigned erow)' has no implementation available\n";
  }
  ~pgmimage();
  void mergeVertically(pgmimage & img, char position='b') { // position = [t|b]
    cerr << "ERROR: function 'void mergeVertically(pgmimage & img, char position='b')' has no implementation available\n";
  } 
  void read(istream & fitx);
  void write(ostream & fitx, bool binary, int irow=0, int erow=0);
};

#endif
