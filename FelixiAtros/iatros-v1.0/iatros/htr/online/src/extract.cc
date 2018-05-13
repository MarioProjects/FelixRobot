/*
 *  main.cc
 *
 *  Created on: Feb 12, 2006
 *  Authors: atoselli@iti.upv.es and moises@iti.upv.es
 */

#include <unistd.h>
#include <version.h>
#include <iostream>
#include <istream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include "online.h"
#include "read.h"
#include "features.h"
using namespace std;



void usage (char *f, int WS, int NP) {
    cerr << endl;
    cerr << "Usage: " << f << " [-w #int] [-n #int] [-o] [<file> (def. stdin)] [-V]" << endl;
    cerr << endl;
    //  cerr << "  -s #   Slant detection threshold [0-100]% (def 90)" << endl;
    cerr << "  -w #   Specify the Hamming window size for FFT determination in" << endl;
    cerr << "         function of points number inside it (def. "<< WS << ")." << endl;
    cerr << "  -n #   Specify the FFT point numbers to be print out (def. " << NP << ")." << endl;
    cerr << "  -o     Save output to disk (def. stdout)." << endl;
    cerr << "  -V     Print version." << endl;
    cerr << endl;
    cerr <<"NOTE: input file must be in MOTOROLA format." << endl << endl;
    cerr << "Feature Vector Scheme:" << endl;
    cerr << "                       < Y Dx Dy Ax Ay K [ 1AReX 1AImX 2AReX 2AImX ... nAReX nAImX -nAReY -nAImY ... -2AReY -2AImY -1AReY -1AImY ] >" << endl << endl;
    cerr << "   where:" << endl;
    // cerr << "          X: normalized coord x" << endl;
    cerr << "          Y: normalized coord y" << endl;
    cerr << "         Dx: first normalized derivative of X" << endl;
    cerr << "         Dy: first normalized derivative of Y" << endl;
    cerr << "         Ax: second derivative of X" << endl;
    cerr << "         Ay: second derivative of Y" << endl;
    cerr << "          K: curvature" << endl;
    cerr << "      kAReX: X k-th Real Fourier Coef. (positive)" << endl;
    cerr << "      kAImX: X k-th Imaginary Fourier Coef. (positive)" << endl;
    cerr << "     -kAReY: Y k-th Real Fourier Coef. (negative)" << endl;
    cerr << "     -kAImY: Y k-th Imaginary Fourier Coef. (negative)" << endl << endl;
    return;
}


void version() {
    cerr << IATROS_HTR_ONLINE_PROJECT_STRING"\n"IATROS_HTR_ONLINE_BUILD_INFO;
}

int main(int argc, char ** argv) {
    
    char *prog;
    if ((prog=rindex(argv[0],'/'))) prog+=1;
    else prog=argv[0];
    
    string filename;
    int opt;
    
    int WWNP=20;
    int NFFTP=0;
    bool OutFile=false;
    char printRes='n';
    char pre_hat='\0';
    float thresh_slt=90;

    while ((opt=getopt(argc,argv,"p:r:s:t:z:ow:n:Vh")) != -1)
	switch (opt) {
	    case 0: usage(prog,WWNP,NFFTP);
	    case 'p': if (*optarg == 'M' || *optarg == 'H'|| *optarg =='m' || *optarg == 'V' || *optarg=='C')
		printRes=*optarg;
	    else {
		cerr << "ERR: invalid -p option " << *optarg << endl; 
		return(-1);
	    }
		break;
	   //  case 'r': if (*optarg == 'd' || *optarg == 'i')
// 		           pre_hat=*optarg;
// 	              else {
// 		cerr << "ERR: invalid -s option " << *optarg << endl; 
// 		return(-1);
// 	    }
// 		break;
	    case 'o': OutFile=true; break;
		// case 't': thresh_hat=atof(optarg); break;
	    case 'w': WWNP=atoi(optarg); break;
	    case 'n': NFFTP=atoi(optarg); break;
   	    case 's': thresh_slt=atof(optarg);break;
      case 'V': version(); return 1; break;
	    default:   cerr << "ERR: invalid option " << char(opt) << endl;
  	    case 'h': usage(prog,WWNP,NFFTP);return 1;
	};
    
    if (!NFFTP) WWNP=0;
    if (optind<argc) filename.append(argv[optind]);
    ifstream *fd=NULL, aux_fd;
    if (filename=="") filename.append("stdin");
    else {
	aux_fd.open(filename.c_str());
	if (aux_fd.fail()) {
	    cerr << "ERR: file \"" << filename.c_str();
	    cerr << "\" can't be opened\n" << endl;
	    return(-1);
	}
	fd=&aux_fd;
    }
    
    char *fname = new char[filename.size()+strlen("_xxx.fea")+1];
    sentence * mostres;
    int cont_mostres=0;
    while (read_file_moto((fd==NULL?cin:*fd),&mostres,filename)) {
	
        //(*mostres).print_moto();	
	
	// Anulamos puntos repetidos
	sentence * no_rep=(*mostres).anula_rep_points();
        //(*no_rep).print_moto();	
	
	// Filtre mitjana
	sentence * traz_suav=(*no_rep).suaviza_traza();
        //(*traz_suav).print_moto();	
	
	// Interpolamos linealmente entre sí los trazos PenDownn
	//sentence * interp_strokes=(*traz_suav).interpol_strokes_pu();
        //(*interp_strokes).print_moto();	
	
	// Buscamos hat strokes
	//(*mostres[i]).marca_back(thresh_hat);

	// Correcion de slant
// 	double ref=0;
// 	float angle=(*traz_suav).MVPV(thresh_slt,ref);
// 	cerr << "Slant Correction: " << "Angle=" << angle; 
// 	cerr << " " << "Reference=" << ref << endl;
// 	sentence *sent_slt=(*traz_suav).tr_shift(-angle,ref);
        //(*sent_slt).print();	

	// Obtenim els minims locals
// 	float mean,desv;
// 	vector<Point> * min_loc=(*sent_slt).minimos_locales(mean,desv);
// 	// Computamos el angulo del slow
// 	float slope = line_detector(*min_loc);
// 	angle = -atan(-1.0/slope);
// 	if (angle < 0) angle=M_PI + angle;

	// rotamos la imagen
//        sentence * sent_rot=(*sent_slt).rotate(angle);
	sentence * sent_rot=traz_suav;
	//sentence *sent_rot=interp_strokes; // Saltamos el Skew
	//sentence *sent_rot=seg_traz; // Saltamos el Skew

	// imprimimos en formato graph
 	// if(printRes=='H') { // la imagen con hat strokes en formato graph
// 	    cout << (*sent_rot).transcrip << endl;
// 	    for (int s=0; s<(*sent_rot).n_strokes; s++)
// 		if ((*sent_rot).strokes[s].pen_down) {
// 		    if ((*sent_rot).strokes[s].is_hat)
// 			cout << "#m=2,S=1" << endl;
// 		    else
// 			cout << "#m=1,S=1" << endl;
		    
// 		    if (pre_hat!='d' || !(*sent_rot).strokes[s].is_hat)
// 		      for (int i=0; i<(*sent_rot).strokes[s].n_points; i++)
// 		         (*sent_rot).strokes[s].points[i].print();
// 		}
// 	}
// 	else if (printRes=='C') { // diferente color para cada stroke
// 	  cout << (*sent_rot).transcrip << endl;
// 	  int color=1;
// 	  for (int s=0; s< (*sent_rot).n_strokes; s++)
// 	    if ((*sent_rot).strokes[s].pen_down){
	      
// 	      cout << "#m=" << (++color%6)+1 <<",S=1" << endl;
// 	      for (int i=0; i<(*sent_rot).strokes[s].n_points; i++)
// 		(*sent_rot).strokes[s].points[i].print();

// 	    }
// 	}
// 	else if (printRes == 'V') { // colorear la vuelta atras
// 	    cout << (*sent_rot).transcrip << endl;
// 	    int xmax=1;
// 	    bool a_iz=false;
// 	    for (int s=0; s< (*sent_rot).n_strokes; s++)
// 		if ((*sent_rot).strokes[s].pen_down){	
// 		    if (!a_iz)
// 			cout << "#m=1,S=2" << endl;
// 		    else 
// 			cout << "#m=2,S=2" << endl;
				
// 		    for (int i=0; i<(*sent_rot).strokes[s].n_points; i++){
// 			if ((*sent_rot).strokes[s].points[i].x > xmax){
// 			    xmax=(*sent_rot).strokes[s].points[i].x;
// 			    if (!a_iz){
// 				cout << "#m=2,S=2" << endl;
// 				a_iz=true;
// 			    }
// 			} else 	if ((*sent_rot).strokes[s].points[i].x < xmax)
// 			    if (a_iz){
// 				cout << "#m=1,S=2" << endl;
// 				a_iz=false;
// 			    }
// 			(*sent_rot).strokes[s].points[i].print();
// 		    }
// 		}
	//    float media=(*sent_rot).calcuar_media_y();
	//    cout << "#m=3,S=1" << endl;
	//    cout << " 0 "<< media <<endl;
	//    cout << xmax << " "<< media << endl; 
// 	}
// 	else 
	if(printRes=='m') { // imagen procesada en formato motorola
 	    (*sent_rot).print_moto();
	} /*
	else if(printRes == 'M') { // imagen con minimos locales y fitting line
 	    (*traz_suav).print();
	    
	    // minimos locales
 	    int n_p_loc=(*min_loc).size();
 	    for (int m=0; m < n_p_loc; m++){
 		cout << "#m=1,S=2" << endl;
 		(*min_loc)[m].print();
 	    }
	    
	    //fitting line
	    cout << "#m=6,S=2" << endl;
	    cout << (*min_loc)[0].x << "  "<<  (*min_loc)[0].x * slope +mean << endl;
	    cout << (*min_loc)[n_p_loc -1 ].x << "  " << (*min_loc)[n_p_loc -1].x * slope +mean << endl;
	    
	    // pintamos la media
	    cout << "#m=3,S=2" << endl;
	    cout <<  (*min_loc)[0].x <<  "  "<< int(mean+0.5) << endl;
	    cout << (*min_loc)[n_p_loc -1 ].x << "  " <<  int(mean+0.5) << endl;
	    
	    // limite inferior (media + desv)
	    cout << "#m=3,S=2" << endl;
	    cout <<  (*min_loc)[0].x <<  "  "<< int(mean+0.5) +desv << endl;
	    cout << (*min_loc)[n_p_loc -1 ].x << "  " <<  int(mean+0.5)+ desv << endl;	         
 	} */
	
	sentenceF feat;
	feat.calculate_features(*sent_rot,pre_hat,WWNP,NFFTP);

	sprintf(fname,"%s_%d.fea",filename.c_str(),++cont_mostres);

	ostream *fdo;
	if (OutFile) 
	    fdo=new ofstream(fname);
	 else 
	    fdo=&cout;
	
	// V2 format header
	*fdo << "Name       " << fname << endl;
	*fdo << "OrtTrans   " << feat.transcrip << endl;
	*fdo << "Structure  y, dx dy ddx ddy k ";
	for (int c=1; c<=NFFTP*2; c++)
	  *fdo << c<< "AReX " << c <<"AImX ";
	*fdo<< endl;
	*fdo << "NumVect    " << feat.n_frames << endl;
	*fdo << "NumParam   " << feat.frames[0].get_fr_dim() << endl;
	*fdo << "Data" << endl;
	feat.print(*fdo);
	
	if (OutFile) 
	    delete fdo;

	
	//Libera esto en cada iteración del bucle de sentecias
        delete mostres;
	delete no_rep;
	delete traz_suav;
	//delete interp_strokes;
	//delete seg_traz;
	//	delete min_loc;
	//delete sent_slt;
	//delete sent_rot;
    }
    // Lineración final de memoria
    delete [] fname;
    
    if (fd!=NULL) fd->close();
    return 0;
}

