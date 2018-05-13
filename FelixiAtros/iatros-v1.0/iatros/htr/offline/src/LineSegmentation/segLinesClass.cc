#include "segLinesClass.h"
#include <string.h>
#include <strings.h>


using namespace std;


////////////////////////////////////////////
//   IMPLEMENTACION DE METODOS PRIVADOS
////////////////////////////////////////////

void lineSegm::makeHistHorizNorm(int thresh, int margin) {
  int nl=int(rows*margin/100.0), tot=0;
  histHoriz = new float[rows];
  for (int r=0; r<rows; r++) histHoriz[r]=0;
  for (int r=nl; r<rows-nl; r++)
    for (int c=nl; c<cols-nl; c++)
      if ((unsigned(image[r][c]) <= thresh)) {
      histHoriz[r]++;
      tot++;
    }
  //   float sum=0, sum2=0;
  for (int r=0; r<rows; r++) {
    histHoriz[r]=histHoriz[r]/tot;
    // sum+=histHoriz[r];
    // sum2+=histHoriz[r]*histHoriz[r];
  }
  med=1.0/rows;
  //  desv=sqrt(sum2/rows-med*med);
}

void lineSegm::smoothHistHoriz(int smth_widht) {
  float * aux = new float[rows];
  for (int r=0; r<rows; r++) {
    float med=0;
    if (histHoriz[r])   
      for (int s=-smth_widht; s<=smth_widht; s++) {
	if ((r+s)<0 || (r+s)>=rows) continue;
	else med+=histHoriz[r+s];
      }
    aux[r]=med/(smth_widht*2+1);
  }
  delete [] histHoriz;
  histHoriz=aux;
  if (verbosity) {
    cerr << "########################" << endl;
    cerr << "# Horizontal Histogram" << endl;
    cerr << "########################" << endl;
    for (int r=0; r<rows; r++)
      cerr << r << "\t" << histHoriz[r] << endl;
    cerr << "#-------------------------------" << endl;
    //cerr << "# Med=" << med << "   " << "Desv=" << desv << endl;
    cerr << "# Med=" << med << endl;
    cerr << "###############################" << endl;
  }
}

bool lineSegm::isShortLine(int srow, int erow, int thresh) {
  float * histVert = new float[cols];
  int min_width_char=int((erow-srow)*.8); // Set the minimum width of character
  					  // as the 80% of line height
  rlsaLineVert(srow,erow,thresh); // Aply a RLSA before to determine the histogram
  for (int c=0; c<cols; c++) {
    histVert[c]=0;
    for (int r=srow; r<=erow; r++)
      if ((unsigned(image[r][c]) <= thresh)) histVert[c]++;
  }
  //cerr << endl;
  //for (int c=0; c<cols; c++) {
  //  cerr << c << " " << histVert[c] << endl;
  //}
  int min_height_char=(erow-srow)/5;  // set lower baseline for
  				      // character detection
  int width_char=0, max_width_char=0;
  bool flag=false;
  for (int c=0; c<cols; c++)
    if (histVert[c]>min_height_char) {
      if (flag) { width_char++; continue; }
      flag=true;
      if (max_width_char<width_char) max_width_char=width_char;
      width_char=1; 
    } else flag=false;
  if (max_width_char<width_char) max_width_char=width_char;
  //cerr << "max_width_char=" << max_width_char << endl;
  delete [] histVert;
  return (max_width_char>min_width_char);
}

// Detecta zonas manuscritas de la pagina separadas por
// espacios en blanco
void lineSegm::searchTextLines(int min_height, float min_length) {
  float acum;
  int br, er;
  for (int r=0; r<rows; r=er) {
    // Busca comienzo de zona escrita
    for (br=r; br<rows && !histHoriz[br]; br++);
    if (br<rows) acum=histHoriz[br]; else break;
    //cerr << "BLACK area detected in line " << br << endl;
    // Busca fin de zona escrita
    for (er=br+1; er<rows && histHoriz[er]; er++)
      if (acum<histHoriz[er]) acum=histHoriz[er];
    //cerr << "WHITE area detected in line " << er << endl;
    // Verifica si la zona detectada cumple requerimientos
    if ((er-br>=min_height) && (acum>=min_length)) {
      datcomp p(br,er-1);
      clusterlines.push_back(p);
    }
    er++;
  }
  if (verbosity > 1) {
    cerr << "##############################" << endl;
    cerr << "#  Detected Blocks Positions" << endl;
    cerr << "##############################" << endl;
    for (int i=0; i<clusterlines.size(); i++)
      cerr << i << " " << clusterlines[i].st << "\t" << clusterlines[i].fn << endl;
  }
}

void lineSegm::searchValleysAndPics(float min_base_height, int min_space) {
  int s, f, ind_min,ind_max;
  datcomp p;
  for (int i=0; i<clusterlines.size(); i++) {
    f=p.fn=clusterlines[i].st+1;
    while (true) {
      p.st=p.fn-1;
      ind_max=p.st;
      for (s=f; s<clusterlines[i].fn &&
	     !(histHoriz[s-1]>min_base_height &&
	       histHoriz[s+1]<min_base_height); s++)
	if(histHoriz[ind_max]<histHoriz[s]) ind_max=s;	
      p.pc=ind_max;
      if (s==clusterlines[i].fn) {
	p.fn=s;
	if ((p.fn-p.st+1)>=min_space){
	 p.ci=0;
 	 p.cf=cols-1;
	 lines.push_back(p);
	}
	break;
      }
      ind_min=s;
      for (f=s+1; f<clusterlines[i].fn &&
	     !(histHoriz[f-1]<min_base_height &&
	       histHoriz[f+1]>min_base_height); f++)
	if (histHoriz[ind_min]>histHoriz[f]) ind_min=f;
      
      if (f==clusterlines[i].fn) p.fn=f; else p.fn=ind_min;
      if ((p.fn-p.st+1)>=min_space){
	 p.ci=0;
	 p.cf=cols-1;
	 lines.push_back(p);
      }
      p.fn++;
    }
  }
  if (verbosity > 1) {
    cerr << "################################" << endl;
    cerr << "#  Detected Valleys and Pics Positions" << endl;
    cerr << "################################" << endl;
    for (int i=0; i<lines.size(); i++)
      cerr << i << " " << lines[i].st << " " << lines[i].pc << " " << lines[i].fn << " " << lines[i].ci << " " << lines[i].cf << endl;
  }
}



void lineSegm::searchValleysAndPicsForShortLines(float line_height, int thresh) {
  int const lh=int(line_height+.5);
  for (int i=0; i<lines.size(); i++) {
    float auxf=fabs(lines[i].fn-lines[i].st)/line_height;
    int auxi=int(auxf);
    int nun_extra_lines=((auxf-auxi)>.75?auxi+1:auxi);
    if (nun_extra_lines>1) {
      if (verbosity>1) {
        cerr << "\nDetected line " << i << " with " << nun_extra_lines;
        cerr << " possible sublines." << endl;
      }
      bool first_line=true;
      int ls=lines[i].st, lf=lines[i].fn;
      int nl=1; datcomp p(lines[i].st,lines[i].st+lh);
      while (nl<nun_extra_lines) {
	// Busca línea con suficiente masa
        if (!isShortLine(p.st,p.fn,thresh)) {
	  nl++; p.st=p.fn; p.fn=(nl==nun_extra_lines)?lf:p.fn+lh;
          continue;
	}
	nl++; p.st=p.fn; p.fn=(nl==nun_extra_lines)?lf:p.fn+lh;
	// Busca la siguiente línea consecutiva a la anterior con suficiente masa
	while(nl<=nun_extra_lines && isShortLine(p.st,p.fn,thresh)) {
          //cerr << "nl+1=" << nl << " is also strong" << endl;
	  if (verbosity>1) {
	    cerr << "In the big line " << i;
	    cerr << " has been found a new smaller line between ";
	    cerr << "rows " << ls << " and " << p.st << endl;
	  }
          if (first_line) {
	    // 1º caso: utilizo el rango ya creado
	    lines[i].fn=p.st;
            lines[i].pc=lines[i].st+((lines[i].fn-lines[i].st)/2);
            lines[i].ci=0;
            lines[i].cf=cols-1;
	    first_line=false;
	  } else {
	    // 2º caso: creo un nuevo rango y lo añado al conjunto
            p.pc=ls+((p.st-ls)/2);
            p.ci=0;
            p.cf=cols-1;
	    datcomp auxp(ls,p.st,p.pc,p.ci,p.cf);
	    lines.push_back(auxp);
	  }
          ls=p.st; 
	  if (nl==nun_extra_lines) break;
	  nl++; p.st=p.fn; p.fn=(nl==nun_extra_lines)?lf:p.fn+lh;
	}
	// Salto esta línea pxq ya esta chequeada, en el caso que continue 
	// siendo "nl<nun_extra_lines"
	if (nl<nun_extra_lines) {
	  nl++; p.st=p.fn; p.fn=(nl==nun_extra_lines)?lf:p.fn+lh;
	}
      }
      // Agrego el último rango encontrado si lo hubiese 
      p.st=ls;
/*      int ind_max=0;
      for (int s=ls; s<=p.st; s++)
        if(histHoriz[ind_max]<histHoriz[s]&& (histHoriz[s-1]< histHoriz[s] &&
	       histHoriz[s+1]<histHoriz[s])) ind_max=s;	
      p.pc=ind_max;*/
      p.pc=ls+((p.fn-p.st)/2);
      p.ci=0;
      p.cf=cols-1;
      if (!first_line) {
        if (verbosity>1) {
	  cerr << "In the big line " << i;
	  cerr << " has been found a new smaller line between ";
	  cerr << "rows " << ls << " and " << p.fn << endl;
        }
        lines.push_back(p);
      }
    }
  }
}



void lineSegm::printImageSegPics() {
  for (int v=0; v<lines.size(); v++) {
    if (lines[v].st<0 || lines[v].st>=rows || \
        lines[v].fn<0 || lines[v].fn>=rows) {
	cerr << "ERROR: row values limits out of range!!!1" << endl;
	exit(-1);
    }
    for (int j=50;j<80;j++) image[lines[v].pc][j]=0;
    for (int j=100;j<cols;j++) image[lines[v].pc][j]=0;
    for (int j=-5;j<6;j++) image[lines[v].pc+j][74]=0;
    for (int j=-4;j<5;j++) image[lines[v].pc+j][75]=0;
    for (int j=-3;j<4;j++) image[lines[v].pc+j][76]=0;
    for (int j=-2;j<3;j++) image[lines[v].pc+j][77]=0;
    for (int j=-1;j<2;j++) image[lines[v].pc+j][78]=0;
    // Paint in gray the area enclosed between separation lines
 //   for (int i=lines[v].st+1; i<lines[v].fn; i++)
 //     for (int j=0; j<cols; j++)
 //	image[i][j]=image[i][j]<maxval/3?0:image[i][j]-maxval/3;
  }
  write(cout,0);
}

void lineSegm::printImageSegValleys() {
  for (int v=0; v<lines.size(); v++) {
    for (int j=0; j<cols; j++) {
      if (lines[v].st<0 || lines[v].st>=rows || \
          lines[v].fn<0 || lines[v].fn>=rows) {
	cerr << "ERROR: row values limits out of range!!!2" << endl;
	exit(-1);
      }
      // Paint in black the separation lines
      image[lines[v].st][j]=0;
      image[lines[v].fn][j]=0;
      //image[lines[v].pc][j]=0;
    }
    // Paint in gray the area enclosed between separation lines
    for (int i=lines[v].st+1; i<lines[v].fn; i++)
      for (int j=0; j<cols; j++)
 	image[i][j]=image[i][j]<maxval/3?0:image[i][j]-maxval/3;
  }
  write(cout,0);
}

void lineSegm::printImageSeg() {
  for (int v=0; v<lines.size(); v++) {
    for (int j=0; j<cols; j++) {
      if (lines[v].st<0 || lines[v].st>=rows || \
          lines[v].fn<0 || lines[v].fn>=rows) {
	cerr << "ERROR: row values limits out of range!!!2" << endl;
	exit(-1);
      }
      // Paint in black the separation lines
      image[lines[v].st][j]=0;
      image[lines[v].fn][j]=0;
      image[lines[v].pc][j]=255;
    }
    // Paint in gray the area enclosed between separation lines
    for (int i=lines[v].st+1; i<lines[v].fn; i++)
      for (int j=0; j<cols; j++)
 	image[i][j]=image[i][j]<maxval/3?0:image[i][j]-maxval/3;
  }
  write(cout,0);
}

void lineSegm::writeLineSegmOverlap(char *fname, unsigned overlap) {
  int upl, lwl, nl;
  char *ofname = new char[strlen(fname)+strlen("_xx.pgm")+1];
  for (int i=0; i<lines.size(); i++) {
    if (lines[i].st<0 || lines[i].st>=rows || \
        lines[i].fn<0 || lines[i].fn>=rows) {
      cerr << "ERROR: row values limits out of range!!!3" << endl;
      exit(-1);
    }
    sprintf(ofname,"%s_%02d.pgm",fname,i+1);
    ostream *fdo=new ofstream(ofname);
    // Determination of extra lines: nl
    nl=int((lines[i].fn-lines[i].st+1)*overlap/100.0);
    // Save Main Line Body
    upl=lines[i].st-nl; if (upl<0) upl=0;
    lwl=lines[i].fn+nl; if (lwl>=rows) lwl=rows-1;
    write(*fdo,0,upl,lwl);
    delete fdo;
  }
  delete [] ofname;
}


void lineSegm::writeLineSegmDetAsc(char *fname, unsigned thresh) {
  char *ofname = new char[strlen(fname)+strlen("_xx.pgm")+1]; 
  pgmimage *imgUpper=NULL, *imgAux=NULL,*imgAux2=NULL;

  for (int i=lines.size()-1; i>=0; i--) {
    if (lines[i].st<0 || lines[i].st>=rows || \
        lines[i].fn<0 || lines[i].fn>=rows) {
      cerr << "ERROR: row values limits out of range!!!4" << endl;
      exit(-1);
    }
    sprintf(ofname,"%s_%02d.pgm",fname,i+1);
    ostream *fdo=new ofstream(ofname);
 
    lineSegm imgCentral(*this,lines[i].st,lines[i].fn);
    
    // Reference Row determination
    imgCentral.makeHistHorizNorm(thresh);
    float refRow=0;
    for (int r=0; r<imgCentral.rows; r++) refRow+=r*imgCentral.histHoriz[r];

    imgUpper=imgAux;
    if (i>0 && lines[i].st==lines[i-1].fn) {
	// Define a MfSet 
	MfSetPGM mf(imgCentral.image,thresh,0,imgCentral.rows-1,imgCentral.cols);
	mf.verbosity=verbosity;
	int maxR=0, nE=0, nS=mf.numSubSets();
	int *setClasses=(int *)malloc(nS*sizeof(int));
	
	if (verbosity>3) cerr << "\n\nLine ID: " << i+1 << endl;
	// Detect descendent strokes from the upper line image
	//mf.upperChosenSets(setClasses,nE,maxR,imgCentral.rows/2,0,imgCentral.cols-1);
	mf.upperChosenSets(setClasses,nE,maxR,int(refRow)-5,0,imgCentral.cols-1);
	if (verbosity>3) {
	    cerr << "Nº of detetcted subSets: " << nS << endl;
	    cerr << "Nº of subSets touching the upper line to be removed: " << nE << endl;
	    cerr << "Reference Row: " << int(refRow) << "    Max Row value: " << maxR << endl;
	}
	
	imgAux=new pgmimage(maxR+1,imgCentral.cols,imgCentral.maxval);
	
	// Clean the ascendents from the current image and generates a
	// new image with this clean strokes, which will be added to the
	// next image
	for (int row=0; row<=maxR; row++)
	    for (int col=0; col<imgCentral.cols; col++)
		if (unsigned(imgCentral.image[row][col])<=thresh) {
		    int rp=mf.findSet(row*imgCentral.cols+col);
		    for (int k=0; k<nE; k++)
			if (rp==setClasses[k]) {
			    imgAux->image[row][col]=imgCentral.image[row][col];
			    imgCentral.image[row][col]=PGM_MAXVAL;
			    break;
			}
		}
	free(setClasses);
    } else imgAux=NULL;

     //Detect ascendent strokes from the lower line image and delete them
     if(i<lines.size()-1 && lines[i].fn==lines[i+1].st ){
 	MfSetPGM mf(imgCentral.image,thresh,0,imgCentral.rows-1,imgCentral.cols);
 	mf.verbosity=verbosity;
	
 	int minR=imgCentral.rows-1,nE=0, nS=mf.numSubSets();
 	int *setClasses=(int *)malloc(nS*sizeof(int));

 	float refRow2=0;
	float max=0;
        for (int r=0; r<imgCentral.rows; r++)
           if(max<imgCentral.histHoriz[r]){
		refRow2=r;
		max=imgCentral.histHoriz[r];
	   }

        mf.lowerChosenSets(setClasses,nE,minR,int(refRow2)+5,0,imgCentral.cols-1);
 	//mf.lowerChosenSets(setClasses,nE,minR,int(refRow),0,imgCentral.cols-1);
 	if (verbosity>3) {
 	    cerr << "Nº of subSets touching the lower line to be removed: " << nE << endl;
 	    cerr << "Reference Row: " << int(refRow) << "    Min Row value: " << minR << endl;
 	}
 	for (int row=imgCentral.rows-1; row>=minR; row--)
 	    for (int col=0; col<imgCentral.cols; col++)
 		if (unsigned(imgCentral.image[row][col])<=thresh) {
 		    int rp=mf.findSet(row*imgCentral.cols+col);
 		    for (int k=0; k<nE; k++)
 			if (rp==setClasses[k]) {
 			    imgCentral.image[row][col]=PGM_MAXVAL;
 			    break;
 			}
 		}
 	free(setClasses);
     }

    if (imgUpper) {
	imgCentral.mergeVertically(*imgUpper,'b');
	delete imgUpper;
    }
    
     // Detect ascendent strokes that are in the upper line image
     int j=i-1;
     if (i>0 && lines[j].fn==lines[i].st) {
	if (lines[j].st<0 || lines[j].st>=rows || \
	    lines[j].fn<0 || lines[j].fn>=rows) {
	    cerr << "ERROR: row values limits out of range!!!5" << endl;
	    exit(-1);
	}
	// int nrowsj=valleys[j].fn-valleys[j].st+1;
        lineSegm imgSuperior(*this,lines[j].st,lines[j].fn);

	MfSetPGM mf(imgSuperior.image,thresh,0,imgSuperior.rows-1,imgSuperior.cols);
	mf.verbosity=verbosity;
	int minR=imgSuperior.rows-1, nE=0, nS=mf.numSubSets();
	int *setClasses=(int *)malloc(nS*sizeof(int));

	imgSuperior.makeHistHorizNorm(thresh);
 	float refRowSup=0;
	float max=0;
        for (int r=0; r<imgSuperior.rows; r++)
           if(max<imgSuperior.histHoriz[r]){
		refRowSup=r;
		max=imgSuperior.histHoriz[r];
	   }

	if (verbosity>3) cerr << "\n\nLine ID: " << i+1 << endl;
	// Detect ascendent strokes that are in the upper line image
	//mf.upperChosenSets(setClasses,nE,maxR,imgCentral.rows/2,0,imgCentral.cols-1);
        //mf.lowerChosenSets(setClasses,nE,minR,imgSuperior.rows/2,0,imgSuperior.cols-1);
	mf.lowerChosenSets(setClasses,nE,minR,int(refRowSup)+5,0,imgSuperior.cols-1);
        if(minR!=imgSuperior.rows-1) {
//	if (verbosity>3) {	
//	    cerr << "Nº of detetcted subSets: " << nS << endl;
//	    cerr << "Nº of subSets touching the upper line to be removed: " << nE << endl;
//	    cerr << "Reference Row: " << int(refRow) << "    Min Row value: " << minR << endl;
//	}
	
	    imgAux2=new pgmimage(imgSuperior.rows-minR,imgSuperior.cols,imgSuperior.maxval);
	    
	    for (int row=0; row<imgSuperior.rows-minR; row++){
		for (int col=0; col<imgSuperior.cols; col++){
		    if (unsigned(imgSuperior.image[minR+row][col])<=thresh) {
			int rp=mf.findSet((row+minR)*imgSuperior.cols+col);
			for (int k=0; k<nE; k++){
			    if (rp==setClasses[k]) {
				imgAux2->image[row][col]=imgSuperior.image[minR+row][col];
//			    imgCentral.image[row][col]=PGM_MAXVAL;
				break;
			    }
			}
		    }
		}
	    }
	    free(setClasses);
	    if (imgAux2) {
		imgCentral.mergeVertically(*imgAux2,'t');
		delete imgAux2;
	    }
	}
     }
    // Save Main Line Body
    imgCentral.write(*fdo,0);
    delete fdo;
  }
  delete [] ofname;
}



int lineSegm::otsu() {
  float * histGreyLevel=makeHistGreyLevelNorm();
  float mu0, mu1, muT, max_sig_B2=0;
  int max_k=0;
  for (int k=0; k<=maxval; k++) {
    mean_double(histGreyLevel,k,mu0,mu1,muT);
    //prob a priori de la classe 0
    float Pr_C0=0;
    for (int i=0; i<=k; i++) Pr_C0+=histGreyLevel[i];
    // funció objectiu
    //float sig_B2=(Pr_C0)*(1-Pr_C0)*(mu0-mu1)*(mu0-mu1);
    float sig_B2=(Pr_C0)*(muT-mu0)*(muT-mu0)+(1-Pr_C0)*(muT-mu1)*(muT-mu1);
    // optimització
    if ( sig_B2 > max_sig_B2) {
      max_sig_B2 = sig_B2;
      max_k=k;
    }
  }
  delete [] histGreyLevel;
  return max_k;
}


float * lineSegm::makeHistGreyLevelNorm() {
  float * histGreyLevel=new float [maxval+1];
  for (int i=0; i<=maxval; i++) histGreyLevel[i]=0;
  for (int r=0; r<rows; r++)
    for (int c=0; c<cols; c++) 
      histGreyLevel[image[r][c]]++;
  int n_pixels=rows*cols;
  for (int i=0;i<=maxval;i++) histGreyLevel[i]/=n_pixels;
  if (verbosity > 2){
    cerr << "#####################################" << endl;
    cerr << "#  Grey Level Normalized Histogram" << endl;
    cerr << "#####################################" << endl;
    for (int i=0; i<=maxval; i++)
      cerr << i << "  " << histGreyLevel[i] << endl;
  }
  return histGreyLevel;
}


void lineSegm::mean_double(float * h, int k, float & mu1, float & mu2,
			   float & muT) {
  float sum0=0, sum_tot0=0, sum1=0, sum_tot1=0;
  for (int i=0; i<=k; i++) {
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


void lineSegm::rlsaHoriz(int thresh) {
  const int longletra=LONGITUD_MEDIA_LETRA;
  for (int row=0; row<rows; row++) {
    int cont=0;
    bool firstTime=true;
    for (int col=0; col<cols; col++)
      if (image[row][col]<=thresh) {
        if (firstTime) firstTime=false;
        else if (cont<longletra)
	  for (int c2=col-cont; c2<=col; c2++) image[row][c2]=0;
        cont=0;
      } else cont++;
  }
}


void lineSegm::rlsaLineVert(int srow, int erow, int thresh) {
  const int longletra=LONGITUD_MEDIA_LETRA;
  for (int col=0; col<cols; col++) {
    int cont=0;
    bool firstTime=true;
    for (int row=erow; row>=srow; row--)
      if (image[row][col]<=thresh) {
        if (firstTime) firstTime=false;
        else if (cont<longletra)
	  for (int r2=row+cont; r2>=row; r2--) image[r2][col]=0;
        cont=0;
      } else cont++;
  }
}


void lineSegm::readSegmInf(char * ifname) {
  const int TALLA=512;
  char cadena[TALLA];
  datcomp p;
  ifstream fdi(ifname);
  if (!fdi) {
    cerr << "Error:  File \"" << ifname << "\" could not be opened " << endl;
    exit(-1);
  }
  p.pc=0;
  p.ci=0;
  p.cf=0;
  while (fdi.getline(cadena,TALLA)) {
    // deteccion de comentarios
    int i=0;
    while (i<TALLA && cadena[i]==' ') i++;
    if (cadena[i]=='#') {
      //cerr << cadena << endl;
      continue;
    }
    istringstream fichlin(cadena);
    char basura[TALLA], basura2[TALLA];
    fichlin >> basura >> basura2 >> p.st >> p.fn >> p.pc >> p.ci >> p.cf;
    if(p.pc==0){
      //p.pc=p.st+((p.fn-p.st)/2);
      int ind_max=p.st;
      for (int s=p.st; s<p.fn ; s++)
	if(histHoriz[ind_max] < histHoriz[s]) ind_max=s;	
      p.pc=ind_max; 
    }
    if(p.ci==0) p.ci=0;
    if(p.cf==0) p.cf=cols-1;
    lines.push_back(p);
    p.pc=0;
    p.ci=0;
    p.cf=0;
  }
  sort(lines.begin(),lines.end());
  fdi.close();
}

// void lineSegm::readSegmInfandImage(char * ifname,char * imgname) {
//   const int TALLA=512;
//   char cadena[TALLA];
//   datcomp p;
//   ifstream fim(imgname);
//   ifstream fdi;
//   vector<int> picos_img;
//   vector<datcomp> picos_ifname;
//   bool flecha=0;	
// 
//   if (!fim) {
//     cerr << "Error:  File \"" << imgname << "\" could not be opened " << endl;
//     exit(-1);
//   }
// 
//   //Leo los picos de la imagen
//   lineSegm image_pics;
//   image_pics.read(fim);
// 
//   for(int r=0;r<image_pics.rows;r++){
//     flecha=0;
//     for(int c=30;c < 100;c++){
// 	if(image_pics.image[r][c]==0){
// 	  for(int j=1;j<30;j++)if(image_pics.image[r][c+j]==0){flecha=1;}else{flecha=0;break;}
//           if(flecha){for(int j=-5;j<6;j++)if(image_pics.image[r+j][c+24]==0){flecha=1;}else{flecha=0;break;}};
//           if(flecha){for(int j=-4;j<5;j++)if(image_pics.image[r+j][c+25]==0){flecha=1;}else{flecha=0;break;}};
// 	  if(flecha){for(int j=-3;j<4;j++)if(image_pics.image[r+j][c+26]==0){flecha=1;}else{flecha=0;break;}};
//           if(flecha){for(int j=-2;j<3;j++)if(image_pics.image[r+j][c+27]==0){flecha=1;}else{flecha=0;break;}};
//           if(flecha){for(int j=-1;j<2;j++)if(image_pics.image[r+j][c+28]==0){flecha=1;}else{flecha=0;break;}};
//           if(flecha){
//             picos_img.push_back(r);
// 	    break;	 
// 	  }
//         }	
//     }
//    }
//  
//    //Leo los picos que proponia
//    if(ifname!=NULL){
//      fdi.open(ifname);;
//      if (!fdi) {
//       cerr << "Error:  File \"" << ifname << "\" could not be opened " << endl;
//       exit(-1);
//      }
//      while (fdi.getline(cadena,TALLA)) {
//       // deteccion de comentarios
//       int i=0;
//       while (i<TALLA && cadena[i]==' ') i++;
//       if (cadena[i]=='#') {
//         //cerr << cadena << endl;
//         continue;
//       }
//       istringstream fichlin(cadena);
//       char basura[TALLA], basura2[TALLA];
//       fichlin >> basura >> basura2 >> p.st >> p.fn >> p.pc;
//       //cout << p.st << "  " << p.fn << endl;
//       picos_ifname.push_back(p);
//       sort(picos_ifname.begin(),picos_ifname.end());
//     }
//    }
  
 //Si estaba propuesto copio los valles, si no los busco
//   bool existe=0;
//   int anterior=0;
//   for(int i=0;i<picos_img.size();i++){
//     existe=0; 
//     if(ifname!=NULL){
//       for(int j=0;j<picos_ifname.size();j++){
// 	if(picos_img[i]==picos_ifname[j].pc){
// 	    p.st=picos_ifname[j].st;
// 	    p.fn=picos_ifname[j].fn;
// 	    p.pc=picos_ifname[j].pc; 	
// 	    lines.push_back(p);
//             picos_ifname.erase(picos_ifname.begin()+j);
//             existe=1;
//             anterior=p.fn; 
//             break;
// 	}
//        }
//     }	
//     if(existe) continue;
//     p.pc=picos_img[i];
//     int ind_min=picos_img[i];
// 
//     for (int s=picos_img[i];s>=anterior;s--)
//        if(histHoriz[ind_min]>histHoriz[s]) ind_min=s;
//     p.st=ind_min;
// 
//     ind_min=picos_img[i];
//     int siguiente;
//     if(i==picos_img.size()-1){siguiente=rows;}else{siguiente=picos_img[i+1];};	
//     for (int s=picos_img[i]; s<siguiente;s++)
//       if(histHoriz[ind_min]>histHoriz[s]) ind_min=s;
//     p.fn=ind_min;	
// 
//     anterior=p.fn;
//     lines.push_back(p);
//   }
//   sort(lines.begin(),lines.end());
//   fdi.close();
//   fim.close();
// }


float lineSegm::determineLineHeightFFT(int searchFrom) {

  double *in_hist=(double*)fftw_malloc(sizeof(double)*rows);
  // Esto es poco óptimo ARREGLAR
  for (int r=0; r<rows; r++) in_hist[r]=histHoriz[r];

  // Solo se necesita la mitad de espacio de salida = int(rows/2)+1
  fftw_complex *out_hist=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(rows/2+1));
  fftw_plan p_hist;
  p_hist=fftw_plan_dft_r2c_1d(rows, in_hist, out_hist, FFTW_ESTIMATE);

  // Make FFT
  fftw_execute(p_hist);
  if (verbosity>1) {
    cerr << "##############################" << endl;
    cerr << "# Histogram Spectrum (Re,Im)" << endl;
    cerr << "##############################" << endl;
    // The component for r=0 is not printed
    for (int r=1; r<(rows/2+1); r++) {
      //cerr << r*(2*M_PI/rows) << " ";
      cerr << (float)rows/r << " ";
      cerr << scientific << setw(10) << out_hist[r][0] << " ";
      cerr << scientific << setw(10) << out_hist[r][1] << endl;
    }
  }

  int pos=0;
  double mod, max=-1.0;
  for (int r=searchFrom; r<(rows/2+1); r++) {
    mod=out_hist[r][0]*out_hist[r][0]+out_hist[r][1]*out_hist[r][1];
    //cout << r << " " << mod << " " << max << " " << pos << endl;
    if (mod>max) { max=mod; pos=r; }
  }
    
  fftw_destroy_plan(p_hist);
  fftw_free(in_hist);
  fftw_free(out_hist);
  return ((float)rows/pos);
}



////////////////////////////////////////////
//   IMPLEMENTACION DE METODOS PUBLICOS
////////////////////////////////////////////

// usa lista de inicializacion
lineSegm::lineSegm(): pgmimage(), histHoriz(NULL),med(0),desv(0) {}


lineSegm::lineSegm(lineSegm & otra, unsigned irow, unsigned erow): pgmimage(otra,irow,erow), histHoriz(NULL),med(0),desv(0) {} 


lineSegm::~lineSegm() {
  if (histHoriz) delete [] histHoriz;
  //~pgmimage();
}


void lineSegm::writeSegmInf(char * fname) {
  sort(lines.begin(),lines.end());  
  char *ofname = new char[strlen(fname)+strlen("_sl.inf")+1];
  sprintf(ofname,"%s_sl.inf",fname);
  ostream *fdo=new ofstream(ofname);
  *fdo << "# File: " << fname << ".pgm" << endl;
  *fdo << "# Resl: " << cols << " " << rows << endl;
  *fdo << "#####################################################" << endl;
  *fdo << "# Line nº:  start     end     pic   col_ini  col_end#" << endl;
  *fdo << "#####################################################" << endl;
  for (int i=0; i<lines.size(); i++) {
    *fdo << "Line " << setfill('0') << setw(2) << i+1 << ":    ";
    *fdo << setfill(' ') << setw(4) << lines[i].st;
    *fdo << "     " << setw(4) << lines[i].fn;
    *fdo << "     " << setw(4) << lines[i].pc;
    *fdo << "     " << setw(4) << lines[i].ci;
    *fdo << "     " << setw(4) << lines[i].cf << endl;
  }
  delete fdo;
  delete [] ofname;
}


void lineSegm::makeSegLines(char * fname, int overlap, int level, bool divline, bool demo) {

  const unsigned thresh=otsu();
  const int smooth_ws=SMOOTH_FACTOR;
  
  lineSegm aux(*this); // This uses the default copy constructor of lineSegm
  aux.rlsaHoriz(thresh);
  //aux.write(cout,0); exit(-1);
  
  aux.makeHistHorizNorm(thresh,0);
  aux.smoothHistHoriz(smooth_ws);
  float minMass=aux.med*(1.0+level/100.0);
  float lineHeight=aux.determineLineHeightFFT(FFT_SEARCH_FROM);
  //cerr << lineHeight << endl; exit(0);
  const int nlMin=int(lineHeight*PORC_MIN_LINE_HEIGHT/100.0);
  aux.searchTextLines(nlMin,aux.med/2);
  aux.searchValleysAndPics(minMass,nlMin);
  if (divline)
    aux.searchValleysAndPicsForShortLines(lineHeight,thresh);
  //aux.write(cout,0); exit(-1);

  lines=aux.lines;
  lines=aux.lines;
  sort(lines.begin(),lines.end());
  if (demo) printImageSeg();
  else if (overlap>-1) writeLineSegmOverlap(fname,overlap);
  else {
       writeLineSegmDetAsc(fname,thresh);
  }
}


void lineSegm::fileSegLines(char * ifname, char * sfname, char * imgname, int overlap, bool demo) {
     
 // if(imgname==NULL){
    const unsigned thresh=otsu(); 
    const int smooth_ws=SMOOTH_FACTOR;
 
    lineSegm aux(*this); 
    aux.rlsaHoriz(thresh);
   
    aux.makeHistHorizNorm(thresh,0);
    aux.smoothHistHoriz(smooth_ws);
    
    aux.readSegmInf(sfname);
    lines=aux.lines;
 // }else{
 //   const unsigned thresh=otsu(); 
 //   const int smooth_ws=SMOOTH_FACTOR;
 
 //   lineSegm aux(*this); 
 //   aux.rlsaHoriz(thresh);
   
 //   aux.makeHistHorizNorm(thresh,0);
 //   aux.smoothHistHoriz(smooth_ws);
 
 //   histHoriz=aux.histHoriz;  
 //   aux.readSegmInfandImage(sfname,imgname);
 //   lines=aux.lines;
 //   lines=aux.lines;
    sort(lines.begin(),lines.end());
 // }

  if (demo) printImageSeg();
  else if (overlap>-1) writeLineSegmOverlap(ifname,overlap);
  else {
    unsigned thr=otsu();
    writeLineSegmDetAsc(ifname,thr);
 }
}
