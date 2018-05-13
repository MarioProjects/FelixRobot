//#include <istream>
#include "mfSetClass.h"

mfSet::mfSet(int talla) {
  n = talla;
  nSubconjuntos = talla;
  mfs = new int[n];
  for (int i=0; i<n; i++)
    mfs[i] = -1;         // el n de nodos i es -1.
}

mfSet::~mfSet() {
  delete[] mfs;
}

inline int mfSet::find(int x) {
  while (mfs[x] >= 0)
    x = mfs[x];
  return x;
}

int mfSet::findCardinalitat(int x){
  while (mfs[x] >= 0)
    x = mfs[x];
  return mfs[x];
}
void mfSet::merge(int x, int y) {
 
  int rx=find(x);
  int ry=find(y);
  if (rx != ry){
    nSubconjuntos--;
    if (mfs[rx] <= mfs[ry]) {   //  menor o igual cuelga el nodo y del nodo x.
      mfs[rx] += mfs[ry];        // Aumentamos la talla del nodo x.
      mfs[ry] = rx; 
    } else{
      mfs[ry] += mfs[rx];
      mfs[rx] =  ry;            // ponemos x como hijo de y
    }
  }
} 
 
int mfSet::Subconjuntos() {
  return nSubconjuntos;
}

void mfSet::obtenRepresentantes(int **v) {
  int p = 0;
  int * pmfs=mfs;
  for (int i=0; i<n; pmfs++, i++)
    if (*pmfs < 0) {
      v[0][p] = i;
      v[1][p] = *pmfs;
      p++;
    }
  
}

void MfSetPGM::init(int rows, int cols) {
  verbosity=0;
  n = rows*cols;
  nSubSets = n;
  mfs = new mfsData[n];
  // It has been optimized
  mfsData *pt=mfs;
  for (int i=0; i<n;pt++, i++) {
  //  mfs[i].activate=true;
    pt->activate=true;
 //   mfs[i].ind_size = -1;                   // el n de nodos i es -1.
    pt->ind_size = -1;                   // el n de nodos i es -1.
 //   mfs[i].maxRow = mfs[i].minRow = i/cols; // max List Row and min List Row
    pt->maxRow = pt->minRow = i/cols; // max List Row and min List Row
   
  }
}

MfSetPGM::MfSetPGM(int rows, int cols) {
  init(rows,cols);
}

MfSetPGM::MfSetPGM(gray **img, unsigned greyThrs, int irow, int erow, int cols) {
  init(erow-irow+1,cols);
  // Determinamos las componentes conexas.
  for (int row=irow; row<=erow; row++)
    for (int col=0; col<cols; col++)
      if (unsigned(img[row][col])<=greyThrs) {
	int pos=(row-irow)*cols+col;
        //int vN=pos-cols;
        int vNE=pos-cols+1;
        int vE =pos+1;
        int vSE=vE+cols;
        int vS =pos+cols;
        //int vSO=pos+cols -1;
        //int vO =pos-1;
        //int vNO=pos-cols -1;
	if (row>irow && col<(cols-1) && unsigned(img[row-1][col+1])<=greyThrs) merge(pos,vNE);
        if (col<(cols-1) && unsigned(img[row][col+1])<greyThrs) merge(pos,vE);
        if (row<erow && col<(cols-1) && unsigned(img[row+1][col+1])<=greyThrs) merge(pos,vSE);
        if (row<erow && unsigned(img[row+1][col])<=greyThrs) merge(pos,vS);
      } else {
	nSubSets--;
	mfs[(row-irow)*cols+col].activate=false;
      }
}


MfSetPGM::~MfSetPGM() {
  delete [] mfs;
}

int MfSetPGM::findSet(int x) {
  while (mfs[x].ind_size>=0) x=mfs[x].ind_size;
  return x;
}

int MfSetPGM::sizeSet(int x) {
  while (mfs[x].ind_size>=0) x=mfs[x].ind_size;
  return -(mfs[x].ind_size);
}


void MfSetPGM::merge(int x, int y) { 
  int rx=findSet(x);
  int ry=findSet(y);
  if (rx != ry){
    nSubSets--;
    if (mfs[rx].ind_size <= mfs[ry].ind_size) { //menor o igual cuelga
					        //el nodo y del nodo x.
      if (mfs[rx].maxRow<mfs[ry].maxRow)  
	mfs[rx].maxRow=mfs[ry].maxRow;
      if (mfs[rx].minRow>mfs[ry].minRow)  
	mfs[rx].minRow=mfs[ry].minRow;
      mfs[rx].ind_size+=mfs[ry].ind_size;
      mfs[ry].ind_size=rx; 
    } else {
      if (mfs[ry].maxRow<mfs[rx].maxRow)
	mfs[ry].maxRow=mfs[rx].maxRow;
      if (mfs[ry].minRow>mfs[rx].minRow)
	mfs[ry].minRow=mfs[rx].minRow;
      mfs[ry].ind_size+=mfs[rx].ind_size;
      mfs[rx].ind_size=ry;               // ponemos x como hijo de y
    }
  }
} 

 
int MfSetPGM::numSubSets() {
  return nSubSets;
}


void MfSetPGM::upperChosenSets(int *v, int &nE, int &maxRow, int ref, int iP, int eP) {
  int p=nE;
  for (int i=iP; i<=eP; i++) {
    int r=findSet(i);
    if (!mfs[r].activate) continue;
    if (mfs[r].ind_size<0 && mfs[r].maxRow<ref) {
      if (verbosity>3)
        cerr << p << "  Ref-Row:" << ref << "  col:" << i << "  Set-Class:" << r << "  SizeSet:" << -mfs[r].ind_size << "  maxRow:" << mfs[r].maxRow << endl;
      v[p] = r;
      mfs[r].activate=false;
      if (maxRow<mfs[r].maxRow) maxRow=mfs[r].maxRow;
      p++;
    }
    nE=p;
  }
}


void MfSetPGM::lowerChosenSets(int *v, int &nE, int &minRow, int ref, int iP, int eP) {
  int p=nE;
  for (int i=n-(eP-iP+1); i<n; i++) {
    int r=findSet(i);
    if (!mfs[r].activate) continue;
    if (mfs[r].ind_size<0 && mfs[r].minRow>ref) {
      if (verbosity>3)
        cerr << p << "  Ref-Row:" << ref << "  col:" << i << "  Set-Class:" << r << "  sizeSet:" << -mfs[r].ind_size << "  minRow:" << mfs[r].maxRow << endl;
      v[p] = r;
      mfs[r].activate=false;
      if (minRow>mfs[r].minRow) minRow=mfs[r].minRow;
      p++;
    }
    nE=p;
  }
}






// int main() {

//   ifstream ifd("ejemplo2.pgm");
//   pgmimage img;
//   img.read(ifd);

//   cerr << "Image of  " << img.cols <<  " x " << img.rows << endl;
//   unsigned greyThreshold=PGM_MAXVAL-50;// ARREGLAR
//   MfSetPGM mf(img.image,greyThreshold,0,img.rows-1,img.cols); // Definimos un mfset.

//   int maxR, minR;
//   int nE=0, nS = mf.numSubSets();
//   cerr << "Nº of subSets: " << nS << endl;


//   // Determinamos el número de particiones y guardamos en un vector
//   // los representantes de cada clase.

//   int *representantes = (int *)malloc(nS*sizeof(int));
//   mf.upperChosenSets(representantes,nE,maxR=0,img.rows/2+2,0,img.cols-1);
//   mf.lowerChosenSets(representantes,nE,minR=img.rows-1,img.rows/2,(img.rows-1)*img.cols,(img.rows-1)*img.cols+img.cols-1);
//   cerr << "Nº subSets to be removed: " << nE << endl;
//   cerr << "Max Row: " << maxR << "   Min Row: " << minR << endl;

//   pgmimage imgUpper(maxR+1,img.cols,img.maxval);
//   pgmimage imgLower(img.rows-minR,img.cols,img.maxval);

//   for (int row=0; row<=maxR; row++)
//     for (int col=0; col<img.cols; col++)
//       if (unsigned(img.image[row][col])<greyThreshold) {
// 	int rp=mf.findSet(row*img.cols+col);
// 	for (int i=0; i<nE; i++) {
// 	  if (rp==representantes[i]) {
// 	    imgUpper.image[row][col]=img.image[row][col];
// 	    img.image[row][col]=PGM_MAXVAL;
// 	    break;
// 	  }
// 	}
//       }

//   for (int row=minR; row<img.rows; row++)
//     for (int col=0; col<img.cols; col++)
//       if (unsigned(img.image[row][col])<greyThreshold) {
// 	int rp=mf.findSet(row*img.cols+col);
// 	for (int i=0; i<nE; i++) {
// 	  if (rp==representantes[i]) {
// 	    imgLower.image[row-minR][col]=img.image[row][col];
// 	    img.image[row][col]=PGM_MAXVAL;
// 	    break;
// 	  }
// 	}
//       }
    
// //  pgmimage img2(230,img.cols,255);
//   //img.mergeVertically(imgLower,'t');
//   //img.mergeVertically(imgUpper,'b');
//   img.write(cout,0);



// }
