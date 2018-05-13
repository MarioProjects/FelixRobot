/*
 *  online.cc
 *
 *  Created on: Feb 12, 2006
 *  Author: moises@iti.upv.es
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "online.h"

using namespace std;
///////////////////////////////////////
// Funciones generales
///////////////////////////////////////

inline int MAX(int a, int b) {
  if (a>=b) return a;
  else return b;		    
}

inline int MIN(int a, int b) {
  if (a<=b) return a;
  else return b;		    
}

///////////////////////////////////////
// Métodos de la clase "stroke"
///////////////////////////////////////

stroke::stroke(int n_p, bool pen_d, bool is_ht): n_points(n_p), pen_down(pen_d), is_hat(is_ht) {}

float stroke::calcular_y_media_stroke() {
  int suma=0;
  for (int i=0; i<n_points; i++) suma+=points[i].y;
  return suma/(float)n_points;
}

float stroke::calcula_long_stroke() {
  float longitud=0;
  for (int p=1; p<n_points; p++) {
    int dx=points[p-1].x-points[p].x;
    int dy=points[p-1].y-points[p].y;
    longitud+=sqrt(double(dx*dx+dy*dy));
  }
  return longitud;
}

void stroke::print() {
  if(pen_down) cout << "#m=1,S=1" << endl;
  else cout << "#m=2,S=1" << endl;
  for (int i=0; i<n_points; i++) points[i].print(cout);
}

void stroke::print_moto(ostream & fd) {
  fd << n_points << endl << pen_down << endl;
  for (int i=0; i<n_points; i++) points[i].print(fd);
}

int stroke::F_XMIN() {
  int xmin=INT_MAX;
  for (int p=0; p<n_points; p++)
    if (xmin>points[p].x) xmin=points[p].x;
  return xmin;
}

int stroke::F_XMAX() {
  int xmax=INT_MIN;
  for (int p=0; p<n_points; p++)
    if (xmax<points[p].x) xmax=points[p].x;
  return xmax;
}

int stroke::F_XMED() {
  int xmed=0;
  for (int p=0; p<n_points; p++) xmed+=points[p].x;
  return xmed/n_points;
}





///////////////////////////////////////
// Métodos de la clase "sentence"
///////////////////////////////////////

sentence::sentence(string w, int n_s): transcrip(w), n_strokes(n_s) {}

void sentence::print(bool print_pen_up) {
  cout << transcrip << endl; // << n_strokes<< endl;
  for (int i=0; i<n_strokes; i++)
    if (strokes[i].pen_down || print_pen_up) strokes[i].print();
}

void sentence::print_moto(bool print_pen_up) {
  cout << transcrip << endl << n_strokes << endl;
  for (int i=0; i<n_strokes; i++)
    if (strokes[i].pen_down || print_pen_up)
      strokes[i].print_moto(cout);
}

void sentence::print_moto_seg(string & filename, stack<int> & segmentacio){
  int seg,seg_ant=0,n=0,s=0,s_ant=0;
  char fname[500];
  char tran[100];
  istringstream tr(transcrip.c_str());
  filename.erase ( filename.length()-4, 4 );

  while (segmentacio.size()){
	seg=segmentacio.top();
	segmentacio.pop();
	int puntsSeg=seg - seg_ant +1;

	int sumPunts=0;
	while (s<n_strokes && sumPunts<puntsSeg){
	  if (strokes[s].pen_down )
	    sumPunts+=strokes[s].n_points;
	  s++;

	}
	if (abs(puntsSeg - sumPunts) > abs(puntsSeg - (sumPunts - strokes[s-1].n_points))){
	  //	  sumPunts -= strokes[s-1].n_points;
	  s--;
	}
	
	tr >> tran;

	sprintf(fname,"%s_%s_%d.dat",filename.c_str(),tran,s);   
	
	ofstream fd(fname);
	
	fd << tran << endl ;
	fd << s - s_ant << endl;

	//	int suma=0;
	for (int i=s_ant; i < s; i++){
	  // suma+= strokes[i].n_points;
// 	  cout << suma << endl;
	  strokes[i].print_moto(fd);
	}
	//	cout << "seg = " << seg << " punts = " << puntsSeg << " reals = "<<suma << endl;
	seg_ant=seg;
	s_ant=s;
	n++;
	fd.close();
    }
}
 
float sentence::calcuar_media_y() {
  float suma=0;
  for (int s=0; s<n_strokes; s++)
    suma+=strokes[s].calcular_y_media_stroke();
  return suma/n_strokes;
}



// Anula puntos repetidos
sentence * sentence::anula_rep_points() {
  sentence * sent_norep=new sentence(transcrip,n_strokes);
  for (int s=0; s<n_strokes; s++) {
    stroke stroke_norep;
    vector<Point> puntos=strokes[s].points;
    int np=strokes[s].n_points;
    for (int p=0; p<np; p++) {
      if (p<(np-1) && puntos[p]==puntos[p+1]) continue;
      Point point(puntos[p].x,puntos[p].y);
      stroke_norep.points.push_back(point);
    }
    stroke_norep.pen_down=strokes[s].pen_down;
    stroke_norep.n_points=stroke_norep.points.size();
    (*sent_norep).strokes.push_back(stroke_norep);
  }
  return sent_norep;
}

// Suaviza: filtro media
sentence * sentence::suaviza_traza(int cont_size) {
  int sum_x,sum_y;
  sentence * sentNorm=new sentence(transcrip,n_strokes);
  for (int i=0; i<n_strokes; i++) {
    stroke strokeNorm;
    vector<Point> puntos=strokes[i].points;
    int np=strokes[i].n_points;
    for (int p=0; p<np; p++){
      sum_x=sum_y=0;
      for (int c=p-cont_size; c<=p+cont_size; c++)
	if (c<0) {
	  sum_x+=puntos[0].x;
	  sum_y+=puntos[0].y;
	} else if (c>=np) {
	  sum_x+=puntos[np-1].x;
	  sum_y+=puntos[np-1].y;
	} else {
	  sum_x+=puntos[c].x;
	  sum_y+=puntos[c].y;
	}
      Point point(int(sum_x/(cont_size*2+1)),int(sum_y/(cont_size*2+1)));
      strokeNorm.points.push_back(point);
    }
    strokeNorm.pen_down=strokes[i].pen_down;
    strokeNorm.n_points=strokeNorm.points.size();
    (*sentNorm).strokes.push_back(strokeNorm);
  }
  return sentNorm;
}


// Segmentacion de trazas con parámetro L
sentence * sentence::normaliza_traza(float L) {
  sentence * sentNorm=new sentence(transcrip,n_strokes);
  bool redistribute=false;
  
  if (L==0) redistribute=true;
  
  for (int i=0; i<n_strokes; i++) {

    stroke strokeNorm;
    vector<Point> puntos=strokes[i].points;
    int np=strokes[i].n_points;
    double * D=new double[np];

    // calculamos el vector de distancias entre puntos
    D[0]=0;
    for (int j=1; j<np; j++) {
      int DX=(puntos[j].x-puntos[j-1].x);
      int DY=(puntos[j].y-puntos[j-1].y);
      D[j]=D[j-1]+sqrt((double)DX*DX+DY*DY);
    }
    
    int M;
    if (redistribute) {
      M=np;
      L=D[M-1]/(M-1);
    } else M=int(D[np-1]/L+1.5);
    
    // calculamos los nuevos puntos
    // ponemos el punto inicial
    Point p_i(puntos[0].x,puntos[0].y);
    strokeNorm.points.push_back(p_i);
    int n=1;
    
    for (int j=1; j<M-1; j++) {
      while (!((D[n-1]<=j*L) && (j*L<=D[n]))) n++;
      float C;
      if (D[n-1]==D[n]) C=1;
      else C=(j*L-D[n-1])/(D[n]-D[n-1]);
	    
      float TX=puntos[n-1].x+(puntos[n].x-puntos[n-1].x)*C;
      float TY=puntos[n-1].y+(puntos[n].y-puntos[n-1].y)*C;
      
      Point p_nuevo(int(TX+0.5), int(TY+0.5));
      strokeNorm.points.push_back(p_nuevo);
    }
    // ponemos el punto final
    Point p_f(puntos[np-1].x,puntos[np-1].y);
    strokeNorm.points.push_back(p_f);

    strokeNorm.pen_down=strokes[i].pen_down;
    strokeNorm.n_points=strokeNorm.points.size();

    (*sentNorm).strokes.push_back(strokeNorm);
    delete [] D;

  }
  return sentNorm;
}

// Segmnetacion de trazas con parámetro M
sentence * sentence::normaliza_traza(int M) {
  sentence * sentNorm=new sentence(transcrip,n_strokes);
  bool redistribute=false;
  
  if (M==0) redistribute=true;
  
  for (int i=0; i<n_strokes; i++) {

    stroke strokeNorm;
    vector<Point> puntos=strokes[i].points;
    int np=strokes[i].n_points;
    double * D=new double[np];

    // calculamos el vector de distancias entre puntos
    D[0]=0;
    for (int j=1; j<np; j++) {
      int DX=(puntos[j].x-puntos[j-1].x);
      int DY=(puntos[j].y-puntos[j-1].y);
      D[j]=D[j-1]+sqrt((double)DX*DX+DY*DY);
    }
    
    if (redistribute) M=np;
    float L=D[np-1]/(M-1);
    
    // calculamos los nuevos puntos
    // ponemos el punto inicial
    Point p_i(puntos[0].x,puntos[0].y);
    strokeNorm.points.push_back(p_i);
    int n=1;
    
    for (int j=1; j<M-1; j++) {
      while (!((D[n-1]<=j*L) && (j*L<=D[n]))) n++;
      float C;
      if (D[n-1]==D[n]) C=1;
      else C=(j*L-D[n-1])/(D[n]-D[n-1]);
	    
      float TX=puntos[n-1].x+(puntos[n].x-puntos[n-1].x)*C;
      float TY=puntos[n-1].y+(puntos[n].y-puntos[n-1].y)*C;
      
      Point p_nuevo(int(TX+0.5), int(TY+0.5));
      strokeNorm.points.push_back(p_nuevo);
    }
    // ponemos el punto final
    Point p_f(puntos[np-1].x,puntos[np-1].y);
    strokeNorm.points.push_back(p_f);

    strokeNorm.pen_down=strokes[i].pen_down;
    strokeNorm.n_points=strokeNorm.points.size();

    (*sentNorm).strokes.push_back(strokeNorm);
    delete [] D;

  }
  return sentNorm;
}


// Interpola linealmente las transiciones PEN-UP producinedo un único trazo
sentence * sentence::interpol_strokes_pu() {
  sentence * sent_intrp=new sentence(transcrip,1);
  
  // Determina la longitd promedio entre puntos de toda la muestra
  float Lt=0; int Npt=0;
  for (int s=0; s<n_strokes; s++) {
    if (!strokes[s].pen_down) continue;
    Npt+=(strokes[s].n_points-1);    
    Lt+=strokes[s].calcula_long_stroke();
  }
  Lt/=Npt;
  
  stroke strokeInterp;
  int si;
  for (si=0; !strokes[si].pen_down && si<n_strokes; si++);
  if (si==n_strokes) return NULL;
  strokeInterp=strokes[si];

  int sf=si+1;
  for (; sf<n_strokes; sf++) {
    if (!strokes[sf].pen_down) continue;
    int sinp=strokes[si].n_points;
    Point pi=strokes[si].points[sinp-1];
    Point pf=strokes[sf].points[0];
    int dx=pf.x-pi.x;
    int dy=pf.y-pi.y;
    double D=sqrt(float(dx*dx+dy*dy));

    int np=int(D/Lt+1.5);	  
    for (int p=1; p<np-1; p++) {
      float C;
      if (D==0) C=1;
      else C=p*Lt/D;
      float TX=pi.x+dx*C;
      float TY=pi.y+dy*C;
      Point p_nuevo(int(TX+0.5),int(TY+0.5));
      strokeInterp.points.push_back(p_nuevo);
    }
    // Copia el trazo final completo
    for (int p=0; p<strokes[sf].n_points; p++) 
      strokeInterp.points.push_back(strokes[sf].points[p]);
    si=sf;
  }
  strokeInterp.pen_down=1;
  strokeInterp.n_points=strokeInterp.points.size();
  (*sent_intrp).strokes.push_back(strokeInterp);

  return sent_intrp;
}


vector<Point> * sentence::minimos_locales(float & mean, float & desv ){
  vector<Point> *min_loc= new vector<Point>;
  int cont_i,cont_ii,cont_d,cont_dd;
  
  for (int s = 0; s < strokes.size(); s++) {
    if (strokes[s].pen_down){
      vector<Point>  puntos=strokes[s].points;
	
      for (int p = 0; p < puntos.size(); p++) {
	cont_ii=MAX(0,p-2);
	cont_i =MAX(0,p-1);
	cont_d =MIN(puntos.size() - 1,p+1);
	cont_dd=MIN(puntos.size() - 1,p+2);
	if (puntos[cont_ii].y <= puntos[ cont_i].y &&  
	    puntos[ cont_i].y <= puntos[p].y &&
	    puntos[p].y >= puntos[cont_d].y && 
	    puntos[cont_d].y >= puntos[cont_dd].y )
	  (*min_loc).push_back(puntos[p]);
      }
    }
  }
  //media y varianza
  double sum=0,sum2=0;
  int max_y=(*min_loc)[0].y,min_y=(*min_loc)[0].y;
  for (int ml=0 ; ml < (*min_loc).size(); ml++){
    if (max_y < (*min_loc)[ml].y) max_y=(*min_loc)[ml].y;
    if (min_y > (*min_loc)[ml].y) min_y=(*min_loc)[ml].y;
    sum+=(double) (*min_loc)[ml].y;
    sum2+= (double) (*min_loc)[ml].y *  (*min_loc)[ml].y;
  }
  sum-= (max_y + min_y);
  sum2-=(min_y*min_y + max_y*max_y);
  mean=sum / ((*min_loc).size() -2);	
  desv = sqrt(sum2/((*min_loc).size()-2) - mean *mean);
  
  // quitamos los puntos que esten alejados de la media
  vector<Point> * aux =new vector<Point>;
  for (int ml=0 ; ml < (*min_loc).size(); ml++)
    if ((*min_loc)[ml].y >= mean  && (*min_loc)[ml].y < mean +desv)
      (*aux).push_back((*min_loc)[ml]);
  delete min_loc;
  min_loc=aux;
  return min_loc;
}


sentence * sentence::rotate(float angle) {
  float c_angle=cos(angle);
  float  s_angle=sin(angle);
  sentence * sentRotated=new sentence(transcrip,n_strokes);
  for (int s = 0; s < strokes.size(); s++) {
    vector<Point>  puntos= strokes[s].points;
    stroke strokeRotated;
    for (int p = 0; p < puntos.size(); p++) {
      int x = int( c_angle*(puntos[p].y)+s_angle*(puntos[p].x)+0.5);
      int y = -int(-s_angle*(puntos[p].y)+c_angle*(puntos[p].x)+0.5);
      Point point(x,y);
      strokeRotated.points.push_back(point);   
    }
    strokeRotated.pen_down=strokes[s].pen_down;
    strokeRotated.n_points=strokeRotated.points.size();
    (*sentRotated).strokes.push_back(strokeRotated);
  }
  return sentRotated;
}


void sentence::marca_back(float thres) {
  int s=0, SI, SF;

  unsigned int suma=0;
  for (int s=0; s<n_strokes; s++)
    suma+=abs(strokes[s].F_XMAX()-strokes[s].F_XMIN());
  float med_str = suma/float(n_strokes);

  //cerr << "Nº of strokes: " << n_strokes << endl;

  // Suponemos que el primer stroke (con pen_down) no es un "hat"
  for ( ; s<n_strokes && !(strokes[s].pen_down); s++);
    //cerr << "NO ES PENDOWN: " << s << endl;
  if (s<n_strokes) {SI=s; s++;} else return;
  //cerr << "ES PENDOWN: " << SI << endl;
  //cerr << "NO ES HAT: " << SI << endl;

  while (s<n_strokes) {
    for ( ; s<n_strokes && !(strokes[s].pen_down); s++);
      //cerr << "NO ES PENDOWN: " << s << endl;
    if (s<n_strokes) {SF=s; s++;} else break;
    //cerr << "ES PENDOWN: " << SF << endl;
    // Suponemos que los "hat" se trazan con posterioridad a sus 
    // strokes correspondientes
    while ( (strokes[SI].F_XMAX()-strokes[SF].F_XMED()) > thres*med_str/2 ) {
      strokes[SF].is_hat=true;
      //cerr << "ES HAT " << SF << endl;
      for ( ; s<n_strokes && !(strokes[s].pen_down); s++);
        //cerr << "NO ES PENDOWN: " << s << endl;
      if (s<n_strokes) {SF=s; s++;} else break;
      //cerr << "ES PENDOWN: " << SF << endl;
    } 
    //cerr << "NO ES HAT: " << SF << endl;
    SI=SF;
  } 
  return;
}


// SLANT CORRECTION
float sentence::MVPV(float thresh_slt, double &ymin) {
  vector<Point> puntos;
  double ymax=0, xmax=0, xmin=DBL_MAX;
  int ** VProj, dim_prj;

  float var_max, var[91], sum, cont;
  int a, a_max;

  ymin=DBL_MAX;

  // Considera solo los trazos PenDown y que no son HATs
  for (int s=0; s<n_strokes; s++)
    if (strokes[s].pen_down && !strokes[s].is_hat) 
      for (int p=0; p<strokes[s].n_points; p++) {
	Point pt=strokes[s].points[p];
	puntos.push_back(pt);
      }
    
  // Calculamos el maximo y mimimo x e y
  for (int i = 0; i < puntos.size(); i++) {
    if ( puntos[i].y < ymin) ymin = puntos[i].y;
    if ( puntos[i].y > ymax) ymax = puntos[i].y;
    if ( puntos[i].x < xmin) xmin = puntos[i].x;
    if ( puntos[i].x > xmax) xmax = puntos[i].x;
  }

  // Dimensión del vector de proyección
  dim_prj=(int)((xmax-xmin)+2*(ymax-ymin));

  // Determinamos los histogramas de proyecciones para todos los angulos
  VProj=get_projections(puntos, xmin, xmax, ymin, ymax);

  /* Calculamos las varianzas de las proyecciones */
  /* y nos quedamos con el angulo que da el maximo */
  var_max=var_VProjection(VProj[0],dim_prj);
  var[0]=var_max;
  a_max=-45; /* ja veurem com ho arreglem */
  for (a=-44; a<=45; a++) {
    var[a+45]=var_VProjection(VProj[a+45],dim_prj);
    if (var_max < var[a+45]) {
      var_max=var[a+45];
      a_max=a;
    }
  }

  //    for (a=-45;a<=45;a++)
  //    fprintf(stderr,"%d %f\n",a,var[a+45]);

  /* calculamos la media ponderada de todas las varianzas (centro de masas)
     que esten por debajo de un procentaje (threshold) del maximo */
  sum=0; cont=0;
  for (a=-45; a<=45; a++) {
    if (var[a+45] >= (thresh_slt/100.0)*var_max) {
      sum+=var[a+45]*a;
      cont+=var[a+45];
    }
  }

  /* Liberamos la memoria de las proyectiones */
  for (a=0;a<=90;a++)
    free(VProj[a]);
  free(VProj);

  return(-sum/cont);
}

sentence * sentence::tr_shift(float angle, double ref) {
  sentence * sentSlant=new sentence(transcrip,n_strokes);

  for (int s=0; s<strokes.size(); s++) {
    vector<Point> puntos=strokes[s].points;
    stroke strokeSlanted;
    for (int p=0; p<puntos.size(); p++) {
      
      float desp=(float)(puntos[p].y-ref)*tan((float)angle*M_PI/180.0);
      int x = int(puntos[p].x+desp+0.5);
      int y = puntos[p].y;

      Point point(x,y);
      strokeSlanted.points.push_back(point);      
    }
    strokeSlanted.pen_down=strokes[s].pen_down;
    strokeSlanted.n_points=strokeSlanted.points.size();
    (*sentSlant).strokes.push_back(strokeSlanted);
  }
  
  return sentSlant;
}


int ** sentence::get_projections(vector<Point> &points, double xmin,
  double xmax, double ymin, double ymax) {

  int rows=(int)(ymax-ymin);
  int cols=(int)(xmax-xmin);
  int dim_prj=(int)(cols+2*rows);
  int despl_inicial=(int)(rows-xmin);

  int desp[91];
  int ** VProj;

  /* Pedimos memoria para las proyecciones */
  VProj= new int *[91];
  for (int i=0; i<=90; i++) 
    VProj[i]= new int [dim_prj];

  /* Inicializamos las proyecciones */
  for (int i=0;i<=90;i++)
    for (int col=0; col < dim_prj; col++) VProj[i][col]=0;

  /* Calculamos las proyecciones */
  for (int a=0; a<=90; a++) 
    for (int i=0; i<points.size(); i++) {
      desp[a]=(int)((points[i].y-ymin)*tan((float)(a-45)*M_PI/180.0));      
      if ((despl_inicial+points[i].x+desp[a]) >= 0 && \
	  (despl_inicial+points[i].x+desp[a]) < dim_prj)
	VProj[a][despl_inicial+points[i].x+desp[a]]++;
    }
  return VProj;
}

inline float sentence::var_VProjection(int *VPr, int cols) {
  float sum=0, sum2=0, var, mean;

  for (int i=0; i<cols; VPr++,i++){
    sum+= (float)(*VPr);
    sum2+= (float)(*VPr)*(*VPr);
  }
  mean = sum/cols;
  var = sum2/cols - mean * mean;
 
  //  fprintf(stderr,"#Vertical Projection\n");
  //  for (i=0;i<cols;i++) fprintf(stderr,"%d %d\n",i, V_Proj[i]);
 
  return sqrt(var);
}

