/*
 *  features.cc
 *
 *  Created on: Feb 12, 2006
 *  Authors: atoselli@iti.upv.es and moises@iti.upv.es
 */

#include "features.h"

//////////////////////////////////////////////////
//   Implementación de métodos de frame
//////////////////////////////////////////////////

void frame::print(ostream & fd) {
    // fd << scientific << setw(10) << x << " ";
  fd << scientific << setw(10) << y << " ";
  fd << scientific << setw(10) << dx << " ";
  fd << scientific << setw(10) << dy << " ";
  fd << scientific << setw(10) << ax << " ";
  fd << scientific << setw(10) << ay << " ";
  fd << scientific << setw(10) << k << " ";
  //fd << scientific << setw(10) << ang << " ";
  for (int i=0; i<fft_feat.size(); i++)
    fd << scientific << setw(10) << fft_feat[i] << " ";
  fd << endl;
}

int frame::get_fr_dim() {
  //return 8+fft_feat.size();	// incorpora x(t) y el ángulo
  return 6+fft_feat.size();	
  //return 7+fft_feat.size();	// incluyendo x(t)
  //return fft_feat.size();	// solo las DFTs
}



//////////////////////////////////////////////////
//   Implementación de métodos de sentenceF
//////////////////////////////////////////////////

sentenceF::sentenceF(): transcrip(""), n_frames(0), frames(NULL) {};

sentenceF::~sentenceF() {
  delete [] frames;
}

bool sentenceF::data_plot(ostream & fd) {
  if (fd.fail()) return false;
  for (int i=0; i<n_frames; i++) {
    // w es la frecuencia para cada FFT(w)
    //int w=(i-n_frames/2);
    //fd << w << '\t';
    float w=(i-n_frames/2)*(2*M_PI/n_frames);
    fd << setprecision(10) << w << '\t';
    // se hace una rotación con el subíndice j
    int j=(i+n_frames/2+n_frames%2)%n_frames;
    frames[j].print(fd);
    fd << endl;
  }
  return true;
}

bool sentenceF::print_FFT(ostream & fd, int C) {
  if (fd.fail()) return false;
  if (C==0) C=n_frames/2;
  int Pos=n_frames/2;
  for (int i=Pos-C; i<=Pos+C; i++) {
    // Descarto la frecuencia cero (media)
    if (i==Pos || i<0 || i>=n_frames) continue;
    frames[i].print(fd);
  }
  return true;
}

bool sentenceF::print(ostream & fd) { 
  if (fd.fail()) return false;
  for (int i=0; i<n_frames; i++) frames[i].print(fd);
  return true;
} 


void sentenceF::calculate_features(sentence &S, char pre_hat, int width_w, int nfft) {
    vector<Point> points;

    if (pre_hat=='i') {
      
      // Busca los trazos HATS y los almacena en list_strpu
      int list_strpu[MAXNUMHATS], list_medX[MAXNUMHATS], n_strpu=0;
      for (int s=0; s<S.n_strokes; s++)
        if (S.strokes[s].pen_down && S.strokes[s].is_hat) {
	  // Mas de 200 trazos HATs en una palabra no es un caso muy frecuente
	  if (n_strpu >= MAXNUMHATS) break;
	  list_strpu[n_strpu]=s;
	  list_medX[n_strpu++]=S.strokes[s].F_XMED();
        }

      // Se ordenan los trazos HATS en función de su centro de masa en X
      for (int i=0; i<n_strpu-1; i++)   /* Invariant: ?                   */   
	for (int j=n_strpu-1; j>i; j--) 
	  if (list_medX[j] < list_medX[j-1]) {
	    int tmp = list_strpu[j];
	    list_strpu[j] = list_strpu[j-1];
	    list_strpu[j-1] = tmp;
	    tmp = list_medX[j];
	    list_medX[j] = list_medX[j-1];
	    list_medX[j-1] = tmp;
	  }
      
      // Comprueba resultados de ordenación    
      //     for (int f=0;f<n_strpu;f++)
      //       cout << n_strpu << " " << f << " " << list_strpu[f] << " " << list_medX[f] << endl;
      //     exit(-1);
      
      
      // ponemos todos los puntos de trazos visibles en un vector
      int cont_nstr=0;
      for (int s=0; s<S.n_strokes; s++)
	if (S.strokes[s].pen_down && !S.strokes[s].is_hat)
	  for (int p=0;p<S.strokes[s].n_points;p++) {
	    Point pt=S.strokes[s].points[p];
	    
	    // Inserta los strokes HAT
	    if ( cont_nstr < n_strpu && pt.x > list_medX[cont_nstr] + OFFSET_INS ) {
	      // Marca como último punto de la traza el punto
	      // almacenado anteriormente --> point_pu=1
	      if (points.size()) points[points.size()-1].setpu();

	      for (int pi=0;pi<S.strokes[list_strpu[cont_nstr]].n_points;pi++) {
		Point pt_ins=S.strokes[list_strpu[cont_nstr]].points[pi];
		if (pi == (S.strokes[list_strpu[cont_nstr]].n_points-1))
		  pt_ins.setpu();
		points.push_back(pt_ins);
	      }
	      cont_nstr++;
	    } 
	    
	    // Marca el último punto de cada traza --> point_pu=1
	    if (p == (S.strokes[s].n_points-1)) pt.setpu();
	    points.push_back(pt);
	    
	  }
      
    } else {
      
      for (int s=0; s<S.n_strokes; s++)
	if (S.strokes[s].pen_down) 
	  if (pre_hat!='d' || !S.strokes[s].is_hat)
	    for (int p=0;p<S.strokes[s].n_points;p++) {
	      Point pt=S.strokes[s].points[p];
	      // Marca el último punto de cada trazo --> point_pu=1
	      if (p == (S.strokes[s].n_points-1)) pt.setpu();
	      points.push_back(pt);
	    }
      
    }


    
    // Normaliza aspecte
    vector<PointR> pointsN=normalizaAspect(points);
    points.clear(); //Liberamos la memoria de los points

    transcrip.append(S.transcrip);
    n_frames=pointsN.size();
    // reservamos memoria para los frames
    frames = new frame[n_frames];

    // tomamos la "x" y la "y" nomalizadas como primeros atributos
    for (int i=0; i<n_frames; i++) {
      frames[i].x=pointsN[i].x;
      frames[i].y=pointsN[i].y;
    }

    // derivadas y ángulo
    calculate_derivatives(pointsN);
    
    // kurvatura
    calculate_kurvature();
    
    // FFTs
    calculate_FFT_feat(pointsN,width_w,1,nfft);

    pointsN.clear();
    
}


///////////////////////////////////
// Métodos privados de sentenceF
///////////////////////////////////

// Normalización de la señal
vector<PointR> sentenceF::normalizaAspect(vector<Point> & puntos) {
  // DBL_MIN ... dan problemas!!!!
  //double ymax=DBL_MIN, xmax=DBL_MIN, ymin=DBL_MAX, xmin=DBL_MAX;
  double ymax=-100000, xmax=-100000, ymin=100000, xmin=100000;
  //cout << xmax << " " << xmin << " " << ymax << " " << ymin << " " << endl;
  
  // calculamos el maximo y mimimo x e y
  for (int i=0; i<puntos.size(); i++) {
    if (puntos[i].y<ymin) ymin=puntos[i].y;
    if (puntos[i].y>ymax) ymax=puntos[i].y;
    if (puntos[i].x<xmin) xmin=puntos[i].x;
    if (puntos[i].x>xmax) xmax=puntos[i].x;
  }
  // Cuando sucede que ymin=ymax ... para el caso de los "-" y los "." p.ej.
  if (ymin < (ymax+.5) && ymin > (ymax-.5)) ymax=ymin+1;
  //cout << xmax << " " << xmin << " " << ymax << " " << ymin << " " << endl;

  vector<PointR> trazoNorm;
  for (int i = 0; i < puntos.size(); i++) {
    const float TAM=100; 
    
    // Comentar para no normalizar
    PointR p(TAM * ((puntos[i].x - xmin)/(ymax - ymin)),TAM * (puntos[i].y - ymin)/(ymax - ymin));
    //PointR p(TAM * ((puntos[i].x - xmin)/(double)(ymax - ymin)),TAM - TAM * (puntos[i].y - ymin)/(double)(ymax - ymin));
    //PointR p(puntos[i].x,puntos[i].y);
    
    // Establece en "PointR p" el atributo de ultimo punto del trazo
    if (puntos[i].getpu()) p.setpu();
    trazoNorm.push_back(p);
  }
  return trazoNorm;
}


// HTK style derivatives
void sentenceF::calculate_derivatives(vector<PointR> & points, bool norm) {
  unsigned int sigma=0;
  const int tamW=2;
  // calculo del denominador
  for (int i=1; i<=tamW; i++) sigma+=i*i;
  sigma=2*sigma;

  // calculo de la 1º derivada
  for (int i=0; i<points.size(); i++) {
    frames[i].dx=0;
    frames[i].dy=0;
    for (int c=1; c<=tamW; c++) {
      double context_ant_x,context_ant_y,context_post_x,context_post_y;
      if (i-c<0) { // punto inferior
	context_ant_x=points[0].x;
	context_ant_y=points[0].y;
      } else {
	context_ant_x=points[i-c].x;
	context_ant_y=points[i-c].y;
      }  
      if (i+c>=points.size()) {
	context_post_x=points[points.size()-1].x;
	context_post_y=points[points.size()-1].y;
      } else {
	context_post_x=points[i+c].x;
	context_post_y=points[i+c].y;
      }
      frames[i].dx+=c*(context_post_x-context_ant_x)/sigma;
      frames[i].dy+=c*(context_post_y-context_ant_y)/sigma;
    // Derivados normalizacion en mala pos del algoritmo
    // ---------------------------------------------------
    if (norm) {
      double module=sqrt(frames[i].dx*frames[i].dx+frames[i].dy*frames[i].dy);
      if (module>0) {
        frames[i].dx /= module;
	frames[i].dy /= module;
      }
    }
    // ---------------------------------------------------
    }
    // Calcula el ángulo
    frames[i].ang=atan2(frames[i].dy,frames[i].dx);
    if (fabs(frames[i].dx)<FLT_MIN) frames[i].dx=0.0;
    if (fabs(frames[i].dy)<FLT_MIN) frames[i].dy=0.0;
  }
  
  // calculo de la aceleración
  for (int i=0; i<points.size(); i++) {
    double context_ant_dx,context_ant_dy,context_post_dx,context_post_dy;
    frames[i].ax=0;
    frames[i].ay=0;
    for (int c=1; c<=tamW; c++) {
      if (i-c<0){ // punta inferior
	context_ant_dx=frames[0].dx;
	context_ant_dy=frames[0].dy;
      } else {
	context_ant_dx=frames[i-c].dx;
	context_ant_dy=frames[i-c].dy;
      }
      if (i+c>=points.size()) {
	context_post_dx=frames[points.size()-1].dx;
	context_post_dy=frames[points.size()-1].dy;
      } else {
	context_post_dx=frames[i+c].dx;
	context_post_dy=frames[i+c].dy;
      }
      frames[i].ax+=c*(context_post_dx-context_ant_dx)/sigma;
      frames[i].ay+=c*(context_post_dy-context_ant_dy)/sigma;
    }
    if (fabs(frames[i].ax)<FLT_MIN) frames[i].ax=0.0;
    if (fabs(frames[i].ay)<FLT_MIN) frames[i].ay=0.0;
  }
}


void sentenceF::calculate_kurvature() {
  for (int i=0; i<n_frames; i++) {
    double norma=sqrt(frames[i].dx*frames[i].dx+frames[i].dy*frames[i].dy);
    if (norma==0) norma=1;
    frames[i].k=(frames[i].dx*frames[i].ay-frames[i].ax*frames[i].dy)/(norma*norma*norma);
  }
}


void sentenceF::calculate_FFT_feat(vector<PointR> & pointsN, int width_w, int step, int nfftp) {

  int n_pts_fft=2*width_w+1;
  // declaraciones para FFTs
  double *in_x, *in_y;
  fftw_complex *out_x, *out_y;
  in_x=(double*)fftw_malloc(sizeof(double)*n_pts_fft);
  in_y=(double*)fftw_malloc(sizeof(double)*n_pts_fft);
  // Solo se necesita la mitad de espacio de salida = int(n_pts_fft/2)+1
  out_x=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(width_w+1));
  out_y=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(width_w+1));
  fftw_plan p_x, p_y;
  p_x=fftw_plan_dft_r2c_1d(n_pts_fft, in_x, out_x, FFTW_ESTIMATE);
  p_y=fftw_plan_dft_r2c_1d(n_pts_fft, in_y, out_y, FFTW_ESTIMATE);
  // ventana de Hamming
  float * ham_amp = new float[n_pts_fft];
  for (int j=0; j<n_pts_fft; j++)
    //ham_amp[j]=1;  // Sin Hamming
    ham_amp[j]=0.54-0.46*cos(M_PI*j/width_w);

  // PARA CHEQUEO
  /*for (int i=0; i<n_frames; i+=step) {
    cerr << pointsN[i].x << " ";
    if (pointsN[i].getpu()) cerr << endl << "N_FRAME=" << i << endl << endl;
  }
  cerr << endl << endl;*/

  bool rep_pt=0;
  PointR pt(0,0);
  for (int i=0,l=0; l<n_frames; i+=step,l++) {

    // in[][] es multiplicado por (-1)^i para poner el origen
    // en el centro de la salida: es como mult por e^j(PI*n)
    //in[i][0]=pointsN[i].x*pow(-1,i);
    //in[i][1]=pointsN[i].y*pow(-1,i);

    // Inserta puntos hacia la izquierda de in[width_w]
    for (int j=width_w; j>=0; j--) {
      if (!rep_pt)
        if ((i-width_w+j-1)<0 || pointsN[i-width_w+j-1].getpu()) {
          pt=pointsN[i-width_w+j];
          rep_pt=1;
        }
      // modulación con ventana de Hamming
      if (rep_pt) {
        in_x[j]=ham_amp[j]*pt.x;
        in_y[j]=ham_amp[j]*pt.y;
      } else {
        in_x[j]=ham_amp[j]*pointsN[i-width_w+j].x;
        in_y[j]=ham_amp[j]*pointsN[i-width_w+j].y;
      }
    }
    rep_pt=0;

    // Inserta puntos hacia la derecha de in[width_w]
    for (int j=width_w+1; j<n_pts_fft; j++) {
      if (!rep_pt && pointsN[i-width_w+j-1].getpu()) {
        pt=pointsN[i-width_w+j-1];
        rep_pt=1;
      }
      // modulación con ventana de Hamming
      if (rep_pt) {
        in_x[j]=ham_amp[j]*pt.x;
        in_y[j]=ham_amp[j]*pt.y;
      } else {
        in_x[j]=ham_amp[j]*pointsN[i-width_w+j].x;
        in_y[j]=ham_amp[j]*pointsN[i-width_w+j].y;
      }
    }
    rep_pt=0;

    // PARA CHEQUEO
    /*cerr << "i=" << i << "    " << "punt=" << pointsN[i].x << "   ";
    for (int j=0; j<n_pts_fft; j++) cerr << in[j][0] << " ";
    cerr << endl;
    continue;*/

    fftw_execute(p_x); /* repeat as needed */
    fftw_execute(p_y); /* repeat as needed */

    // Si no se se especifica nfftp se toma todos los puntos de la FFT
    if (nfftp<1 || nfftp>width_w) nfftp=width_w;
    // j=0 no se usa para descartar la Frec=0
    // Se toman los nfftp puntos de la derecha
    for (int j=1; j<=nfftp; j++) {
      // se hace una rotacion con el subíindice j
      //int k=(j+n_pts_fft/2+n_pts_fft%2)%n_pts_fft;
      frames[l].fft_feat.push_back(out_x[j][0]);
      frames[l].fft_feat.push_back(out_x[j][1]);
    }
    // Obtengo la parte negativa (conjugado de la positiva)
    for (int j=nfftp; j>=1; j--) {
      // se hace una rotacion con el subindice j
      //int k=(j+n_pts_fft/2+n_pts_fft%2)%n_pts_fft;
      frames[l].fft_feat.push_back(out_y[j][0]);
      frames[l].fft_feat.push_back(-out_y[j][1]);
    }
  }

  //exit(-1);

  delete [] ham_amp;
  fftw_destroy_plan(p_x); fftw_destroy_plan(p_y);
  fftw_free(in_x); fftw_free(in_y);
  fftw_free(out_x); fftw_free(out_y);
  
}
