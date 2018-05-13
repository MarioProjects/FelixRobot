
/*******************************************************************
*  COMPILAR: gcc -o pgmborde pgmborde.c libpgm.c libpbm.c  libprep.c -O3 -lm
********************************************************************/

#include <version.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "data_sets/libpgm.h"
#include "data_sets/libpbm.h"
#include "data_sets/libprep.h"
#include "pgmborde.h"
/*****************************************************************************/

#define MIN(x,y) ( ((x)<(y)) ? (x) : (y) )
#define MAX(x,y) ( ((x)>(y)) ? (x) : (y) )

/*****************************************************************************/
#define TAMANYO_MINIMO_PALABRA 50
#define LONGITUD_MEDIA_LETRA 40
#define ASCENDENTES 0.3
#define DESCENDENTES 0.15
#define TIPO_NORMALIZACION 5  //media 

/* int rows,cols; */
/* gray imaxval; */
/* float p_asc,p_des; */
/* int m_normalizacion; */
/* int verbosity=0; */
/* int out_ascii=1; */

#ifndef PLUGIN

void version() {
    fprintf(stderr, IATROS_HTR_OFFLINE_PROJECT_STRING"\n"IATROS_HTR_OFFLINE_BUILD_INFO);
}


void usage(char * progName,char * cad){
    fprintf(stderr,"Usage: %s %s\n",progName,cad);
    exit(1);
}


int main(int argc, char *argv[]){

   pgmborde_values vals;
   int option;gray **res;
   vals.ifd=stdin;
   vals.ofd=stdout;
   vals.verbosity = 0;
   vals.blanco = 0;
   vals.l = 0;
   vals.r = 0;
   vals.a = 0;
   vals.d = 0;
    
 

    char usagecad[]="[-i pgm_file][-o pgm_file][-l num][-r num][-v num][-h] \n\
    Options are:\
       -i pgm_file     fichero de entrada (default stdin)\n\
       -o pgm_file     fichero de salida (default stdout)\n\
       -l <num>        %% de la izquierda que ser� borrado (default 0)\n\
       -r <num>        %% de la derecha que ser� borrado (default 0)\n\
       -a <num>        %% de la parte superior que ser� borrado (default 0) \n\
       -d <num>        %% de la parte inferior que ser� borrado (default 0) \n\
       -b              En lugar de eliminar pone los pixeles a blanco \n\
       -v level        verbosidad\n\
       -V              version\n\
       -h              Esta ayuda \n";

    
    while ((option=getopt(argc,argv,"hi:o:l:r:a:d:bV"))!=-1)
	switch (option)
	{
	    case 'i':
		vals.ifd=fopen(optarg,"r");
		break;
	    case 'o':
		vals.ofd=fopen(optarg,"w");
		break;
	    case 'l':
		vals.l=atof(optarg);
		break;
	    case 'r':
		vals.r=atof(optarg); 
		break;
            case 'a':
		vals.a=atof(optarg); 
		break;
	    case 'd':
		vals.d=atof(optarg); 
		break;
	    case 'b':
		vals.blanco=1; 
		break;
	    case 'v':
		vals.verbosity=atoi(optarg);
		break;
	    case 'V':
	    version();	
		break;
	    case 'h':
		usage(argv[0],usagecad);
		break;
	    default:
		usage(argv[0],usagecad);
	}
    
    if (vals.ifd==NULL  || vals.ofd==NULL) {
	fprintf(stderr,"pgmborde: El fichero de entrada y/o salida no puede abrirse o no existe\n");
    exit(-1);
    }

    if((res = pgmborde(&vals,NULL)) != NULL){
      pgm_freearray(res,vals.rows);
      return -1;
   } else return 0;
    
}
#endif
unsigned char **pgmborde(pgmborde_values *vals, gray **input){
  int n_cols,n_rows,desp_l,desp_r,desp_a,desp_d;
  gray **original,**nueva;
  gray imaxval;
  int f,c;

  int out_ascii=1;

  if(input ==NULL){
    original=pgm_readpgm(vals->ifd,&vals->cols,&vals->rows,&imaxval);
    if(vals->ifd!=stdin)   fclose(vals->ifd);
  }
  else{
    original=input;
    imaxval=255;
  }
  
     
    /* inicializar matrices */

    desp_l=(int)(vals->cols*vals->l);
    desp_r=(int)(vals->cols*vals->r);
    desp_a=(int)(vals->rows*vals->a);
    desp_d=(int)(vals->rows*vals->d);
    n_cols=vals->cols-desp_l-desp_r;
    n_rows=vals->rows-desp_a-desp_d;
   
    if(n_cols<1) {
	fprintf(stderr,"pgmborde: El porcentage de derecha o de izquierda es incorrecto");
	return 0;
    }
    if(n_rows<1) {
	fprintf(stderr,"pgmborde: El porcentage de arriba o de abajo es incorrecto");
	return 0;
    }

    if (vals->blanco)  nueva=pgm_allocarray(vals->cols,vals->rows);
    else  nueva=pgm_allocarray(n_cols,n_rows);
   
    if (vals->blanco){
	for(f=0;f<vals->rows;f++){
	    for(c=0;c<vals->cols;c++){
		if ((f < desp_a) || (f >= vals->rows-desp_d) || (c < desp_l) || (c >= vals->cols-desp_r))  nueva[f][c]=imaxval;
		else nueva[f][c]=original[f][c];
	    }
	}
    }else{
	for(f=0;f<n_rows;f++){
	    for(c=0;c<n_cols;c++){
		nueva[f][c]=original[f+desp_a][c+desp_l];
	    }
	}
    }	
    if(vals->ofd!=NULL){
      if(vals->blanco) pgm_writepgm(vals->ofd,nueva,vals->cols,vals->rows,imaxval,out_ascii);
      else   pgm_writepgm(vals->ofd,nueva,n_cols,n_rows,imaxval,out_ascii);
      if(vals->ofd!=stdout) fclose(vals->ofd);
    }

    //pgm_freearray(original,vals->rows);
    if (!vals->blanco){ vals->cols=n_cols;vals->rows=n_rows;}
  
    return nueva;

}

