/* pgmslope_MVPH -
 * 	
 *          Calculates the average slope of a text image
 *
 * Copyright (C) 2004 by Mois�s Pastor i Gadea (mpastorg@dsic.upv.es) 
 *                        PRHLT group
 *****************************************************************************/

/* compilar: gcc -o pgmslope_MVPH pgmslope_MVPH.c ./libpgm.c libprep.c -lm -O3 */
#define UMBRAL_VERTICAL 1

#define VERBOSA 0
#define NO_VERBOSA 1
#define ProuNegre 170
#define ANG_RANGE 20

#include <version.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include "data_sets/libpgm.h"
#include "data_sets/libprep.h"
#include "pgmslope.h"
#include "constants.h"

char *prog;
int rows, cols, verbosity=0;
gray **frase,**rotada,**img_crop;
gray imaxval;
float threshold=1;
int out_ascii=1;
int global=0;




void rotar2(int inicio, int fin, float angle, int despH, int extrcols, int despV) {
  float original_row, original_col;
  gray auxLL, auxUL, auxLU, auxUU;
  float auxL, auxU, aux;
  int row, col, rowL, rowU, colL, colU;
  double c_angle, s_angle;

  int row_central = (int)(rows/2.0+.5);
  int col_central = (int)((fin+inicio)/2.0+.5);
  c_angle=cos(angle);
  s_angle=sin(angle);

  for (col=inicio-extrcols/2; col<fin+extrcols/2; col++)
    for (row=0-despV/2; row<rows+despV/2; row++) {
      /* exact sampling location: */
      original_row =  c_angle*(row-row_central) + \
	              s_angle*(col-col_central) + row_central;
      original_col = -s_angle*(row-row_central) + \
	              c_angle*(col-col_central) + col_central;
	
      /* four nearest coefficients to the sampling location: */
      rowL = ((int)original_row);   // round of int
      rowU = ((int)original_row)+1;
      colL = ((int)original_col);   // round of int
      colU = ((int)original_col)+1;
      
      /* Lagrangian interpolation: */
      auxLL = (colL>=inicio && colL<fin && rowL>=0 && rowL<rows)?frase[rowL][colL]:imaxval;
      auxUL = (colL>=inicio && colL<fin && rowU>=0 && rowU<rows)?frase[rowU][colL]:imaxval;
      auxLU = (colU>=inicio && colU<fin && rowL>=0 && rowL<rows)?frase[rowL][colU]:imaxval;
      auxUU = (colU>=inicio && colU<fin && rowU>=0 && rowU<rows)?frase[rowU][colU]:imaxval;
      auxL = (rowU-original_row)*auxLL*(PGM_MAXVAL/imaxval) + \
	     (original_row-rowL)*auxUL*(PGM_MAXVAL/imaxval);
      auxU = (rowU-original_row)*auxLU*(PGM_MAXVAL/imaxval) + \
	     (original_row-rowL)*auxUU*(PGM_MAXVAL/imaxval);
      aux = (original_col-colL)*auxU + (colU-original_col)*auxL;
      rotada[row+despV/2][col+despH]=(gray)(aux+.5);
    }
}


void rotar(int inicio, int fin, float angle) {
  float original_row, original_col;
  gray auxLL, auxUL, auxLU, auxUU;
  float auxL, auxU, aux;
  int row, col, rowL, rowU, colL, colU;
  double c_angle, s_angle;

  int row_central = (int)(rows/2.0+.5);
  int col_central = (int)((fin+inicio)/2.0+.5);
  c_angle=cos(angle);
  s_angle=sin(angle);

  for (col=inicio; col<fin; col++)
    for (row=0; row<rows; row++) {
      /* exact sampling location: */
      original_row =  c_angle*(row-row_central) + \
	              s_angle*(col-col_central) + row_central;
      original_col = -s_angle*(row-row_central) + \
	              c_angle*(col-col_central) + col_central;
	
      /* four nearest coefficients to the sampling location: */
      rowL = ((int)original_row);   // round of int
      rowU = ((int)original_row)+1;
      colL = ((int)original_col);   // round of int
      colU = ((int)original_col)+1;
      
      /* Lagrangian interpolation: */
      auxLL = (colL>=inicio && colL<fin && rowL>=0 && rowL<rows)?frase[rowL][colL]:imaxval;
      auxUL = (colL>=inicio && colL<fin && rowU>=0 && rowU<rows)?frase[rowU][colL]:imaxval;
      auxLU = (colU>=inicio && colU<fin && rowL>=0 && rowL<rows)?frase[rowL][colU]:imaxval;
      auxUU = (colU>=inicio && colU<fin && rowU>=0 && rowU<rows)?frase[rowU][colU]:imaxval;
      auxL = (rowU-original_row)*auxLL*(PGM_MAXVAL/imaxval) + \
	     (original_row-rowL)*auxUL*(PGM_MAXVAL/imaxval);
      auxU = (rowU-original_row)*auxLU*(PGM_MAXVAL/imaxval) + \
	     (original_row-rowL)*auxUU*(PGM_MAXVAL/imaxval);
      aux = (original_col-colL)*auxU + (colU-original_col)*auxL;
      rotada[row][col]=(gray)(aux+.5);
    }
}


inline float var_HProjection(int *HPr, int rows) {
  int j;
  float sum=0, sum2=0, var, mean;
  int *H_Proj=HPr;
  
  for (j=0; j<rows; HPr++,j++){
    sum += (float)(*HPr);
    sum2 += (float)(*HPr)*(*HPr);
  }
  mean=sum/rows;
  var = sum2/rows - mean*mean;

  if (verbosity == 3){
    fprintf(stderr,"#Horizontal Projection\n");
    for (j=0; j<rows; j++) 
      fprintf(stderr,"%d %d\n",j, H_Proj[j]); 
  }
  return sqrt(var);
} 


int ** get_projections(gray ** image, int inicio_tramo, int fin_tramo, int rows){
  int row, col, Ncols, dim_prj;
  int a, i, desp;
  int ** HProj;
  
  Ncols = fin_tramo - inicio_tramo;
  dim_prj = (int)(2*sqrt(Ncols*Ncols+rows*rows/4));

  /* Pedimos memoria para las proyecciones y las inicializamos */
  HProj = (int **)malloc(sizeof(int*)*(2*ANG_RANGE+1));
  for (i=0; i<=2*ANG_RANGE; i++) {
    HProj[i] = (int *)malloc(sizeof(int)*dim_prj);
    for (row=0; row<dim_prj; row++) HProj[i][row]=0; // Inicializaci�n
  }

  /* Calculamos las proyecciones */
  for (col=0; col<Ncols; col++)
    for (row=0; row<rows; row++) 
      if (image[row][inicio_tramo+col] < ProuNegre)  /* lo tenemos en cuenta */
	for (a=0; a<=2*ANG_RANGE; a++) {      /* si es suficientemente negro */
	  desp = (int)((rows/2.0-row)*cos((float)(a-ANG_RANGE)*M_PI/180.0) + \
	    col*sin((float)(a-ANG_RANGE)*M_PI/180.0));
	  if ((dim_prj/2+desp>=0) && (dim_prj/2+desp<dim_prj))
	    HProj[a][dim_prj/2+desp]++;
	}
  
  return HProj;
}


float MVPV(gray ** image, int inicio_tramo, int fin_tramo, int rows) {
  float var_max, var[2*ANG_RANGE+1], sum, cont;
  int a, a_max;
  int ** HProj;
  int Ncols, dim_prj;
  
  //dim_prj=(fin_tramo-inicio_tramo)*4;
  Ncols = fin_tramo - inicio_tramo;
  dim_prj = 2*sqrt(Ncols*Ncols+rows*rows/4);
  // Adquiero las proyecciones entre -45� y 45�
  HProj = get_projections(image,inicio_tramo,fin_tramo,rows);

  /* Calculamos las varianzas de las proyecciones*/
  /* y nos quedamos con el angulo que da el maximo */
  if (verbosity == 3) fprintf(stderr,"#Angulo: -45\n");
  var_max = var_HProjection(HProj[0],dim_prj);
  var[0] = var_max;
  a_max = -ANG_RANGE; /* ja veurem com ho arreglem */
  // sum=0;
  for (a=-ANG_RANGE+1; a<=ANG_RANGE; a++) {
    if (verbosity == 3) fprintf(stderr,"#Angulo: %d\n",a);
    var[a+ANG_RANGE] = var_HProjection(HProj[a+ANG_RANGE],dim_prj);
    if (var_max < var[a+ANG_RANGE]) {
      var_max=var[a+ANG_RANGE];
      a_max=a;
    }
  }
  if (verbosity == 2)
    for (a=-ANG_RANGE; a<=ANG_RANGE; a++)
      fprintf(stderr,"%d %f\n",a,var[a+ANG_RANGE]);
  /* calculamos la media ponderada de todas las varianzas (centro de masas)
     que esten por debajo de un procentaje (threshold) del maximo */
  sum=0;
  cont=0;
  for (a=-ANG_RANGE; a<=ANG_RANGE; a++){
    if (var[a+ANG_RANGE] >= threshold*var_max) {
      sum += var[a+ANG_RANGE] * a;
      cont += var[a+ANG_RANGE];      
    }
  }  
  /* Liberamos la memoria de las proyectiones */
  for (a=0; a<=2*ANG_RANGE; a++) free(HProj[a]);
  free(HProj);

  // Devuelve el valor de SLOPE del segmento en grados
  return(-sum/(1.0*cont));
}


void version() {
    fprintf(stderr, IATROS_HTR_OFFLINE_PROJECT_STRING"\n"IATROS_HTR_OFFLINE_BUILD_INFO);
}


void usage(char * cad){
  fprintf(stderr,"Usage: %s %s\n",prog,cad);
  exit(1);
}

#ifndef PLUGIN

int main (int argc, char *argv[]) {
  pgmslope_values vals;
  int option; gray **res;

  char usagecad[]=" [Options]\n \
          Options are:\n \
                 -i imput_image_file      by default stdin\n \
                 -o output_image_file     by default stdout\n \
                 -t threshold             by default 100%\n \
                 -b                       binary output mode\n \
                 -d                       demostration mode\n \
                 -g                       global image\n \
                 -h                       shows this information\n \
                 -v level                 verbosity mode\n \
                 version\n\n";
  prog=argv[0];
  vals.version=NO_VERBOSA;
  vals.verbosity=0;
  vals.ifd=stdin;
  vals.ofd=stdout;
  vals.threshold=1;
  vals.out_ascii=1;
  vals.global=0;
  while ((option=getopt(argc,argv,"hbgdt:v:i:o:V"))!=-1)
      switch (option)
      {
	  case 'V':
	      version(); 
	      break;
	  case 'i':
	      vals.ifd=fopen(optarg,"r");
	      break;
	  case 'o':
	      vals.ofd=fopen(optarg,"w");
	      break;
	  case 't':
	      vals.threshold=threshold=atof(optarg)/100.0;
	      break;
	  case 'd':
	      vals.version=VERBOSA;
	      break;
	  case 'v':
	      vals.verbosity=verbosity=atoi(optarg);
	      break;
	  case 'b':
	      vals.out_ascii=out_ascii=0;
	      break;
	  case 'g':
	      vals.global=global=1;
	      break;
	  case 'h':
	      usage(usagecad);
	      break;
	  default:
	      usage(usagecad);
      }
  
  if (vals.ifd==NULL || vals.ofd==NULL) {
      fprintf(stderr,"El fichero de entrada o de salida no ha podido abrirse\n");
      exit(-1);
  }
  if((res=pgmslope(&vals,NULL)) !=NULL){
    pgm_freearray(res,vals.rows);
    return -1;
  } else return 0; 
  
}
#endif
gray **pgmslope(pgmslope_values *vals, gray **input){

  int *inicio_tramo, *fin_tramo, n_tramos, tramo;
  int row, col, rows_salida;
  float angulo;
  gray **salida;
  int N_pixels, auxcols, auxcols1=0, auxcols2=0, Ncols=0;
  int Extrcols=0, Extrrows=0;

  if(input ==NULL){
    frase=pgm_readpgm(vals->ifd, &cols, &rows, &imaxval);
    if(vals->ifd!=stdin)
      fclose(vals->ifd);
  }
  else{
    frase=input;
    rows=vals->rows;
    cols=vals->cols;
    imaxval=255;
    threshold=vals->threshold/100.0;
  }
  
  
  
  
  /***************************************
   * Deteccion de tramos 
   ***************************************/

  if(vals->global==0) n_tramos=deteccionTramos(frase,&inicio_tramo,&fin_tramo,rows,cols,imaxval);
  else{
      inicio_tramo=(int *) malloc(sizeof(int));
      fin_tramo=(int*) malloc(sizeof(int));
      n_tramos=1;
      inicio_tramo[0]=0;
      fin_tramo[0]=cols-1;
  }
  /*************************************************
   * Inicializacion de matriz para la imagen rotada
   *************************************************/
 
  if (!vals->global){
    for (tramo=0; tramo<n_tramos; tramo++)
      if (fin_tramo[tramo]-inicio_tramo[tramo]>0) {
	auxcols=(fin_tramo[tramo]-inicio_tramo[tramo]);
	Extrcols+=(sqrt(auxcols*auxcols+rows*rows)-auxcols);
      }
    Extrrows=rows*(sqrt(2)-1); // Para el caso mas desfavorable de 45�
  }else {
    Extrrows=0;
    Extrcols=0;
  }
  rotada=pgm_allocarray(cols+Extrcols,rows+Extrrows);

  /**************************************
   * Procesado de los tramos
   **************************************/ 
  if (vals->verbosity==1)
    fprintf(stderr,"%s: ------- %i Tramos encontrados -------\n",prog,n_tramos);

  for (tramo=0; tramo<n_tramos; tramo++)
    if ((fin_tramo[tramo]-inicio_tramo[tramo] > rows)||(vals->global==1)) {
      if (vals->verbosity)
	fprintf(stderr,"%s: Tramo %d inicio: %d  fin: %d\n",prog,tramo,inicio_tramo[tramo],fin_tramo[tramo]);
      
      /*****************************************
       * Calculo del angulo por maxima varianza 
       * de la proyecci�n vertical
       *****************************************/
      angulo=MVPV(frase,inicio_tramo[tramo],fin_tramo[tramo],rows);

      /********************************************************************
       * Determina espacio extra de cols y los desplazamientos para los
       * segmentos de imagen rotada
       ********************************************************************/
      auxcols = fin_tramo[tramo]-inicio_tramo[tramo];
      if (tramo==0) {
	auxcols1=((auxcols*cos(angulo*M_PI/180.0)+rows*sin(angulo*M_PI/180.0))-auxcols+.5);
	Ncols=auxcols1/2;
	auxcols2=auxcols1;
      } else {
	auxcols2=((auxcols*cos(angulo*M_PI/180.0)+rows*sin(angulo*M_PI/180.0))-auxcols+.5);
	Ncols+=(auxcols2/2 + auxcols1/2);
	auxcols1=auxcols2;
      }
      rotar(inicio_tramo[tramo],fin_tramo[tramo],(-angulo)*M_PI/180.0);
      if (vals->verbosity==1) 
	fprintf(stderr,"%s: Tramo %d: slope medio: %2.2f\n",prog,tramo,angulo);

    } else {

      // Para el caso que los segmentos sean muy cortos: ((fincol-iniciocol)/rows)<1
      if (vals->verbosity)
	fprintf(stderr,"%s: Tramo %d: inicio: %d  fin: %d  --> NO PROCESADO\n",prog,tramo,inicio_tramo[tramo],fin_tramo[tramo]);

      // Determina espacio extra para los segmentos de imagen rotada
      auxcols = fin_tramo[tramo]-inicio_tramo[tramo];
      if (tramo==0) {
	auxcols1=0;
	Ncols=0;
      } else {
	auxcols2=0;
	Ncols+=auxcols1/2;
	auxcols1=auxcols2;
      }
      rotar(inicio_tramo[tramo],fin_tramo[tramo],0);
    }


  // Realiza un crop de las columnas de la derecha
 /*  N_pixels=0; */
/*   for (col=cols+Extrcols-1; col>=0; col--) { */
/*     for (row=0; row<rows+Extrrows; row++) */
/*       if (rotada[row][col] < imaxval*0.5) N_pixels++; */
/*     if (N_pixels > 5) break; */
/*   } */

/*   col+=5; */
/*   if (col>cols+Extrcols) col=cols+Extrcols; */
  
  /* crop de la imagen normalizada */
//  img_crop = crop(rotada, cols+Extrcols, rows+Extrrows, &row, &col,imaxval);
 
  if (vals->version==VERBOSA) {
    salida=pgm_allocarray(col,2*(rows+Extrrows));
    copiar(frase,salida,cols,rows);
    copiar(rotada,salida,col,rows+Extrrows);
    rows_salida=2*rows+Extrrows;
    if(vals->ofd!=NULL){
      pgm_writepgm(vals->ofd,salida,col,rows_salida,imaxval,vals->out_ascii);
      if(vals->ofd!=stdout)
        fclose(vals->ofd);
      vals->rows=rows_salida;
      vals->cols=col;
    }
    pgm_freearray(frase,rows);
    pgm_freearray(rotada,rows+Extrrows);
    free(inicio_tramo); free(fin_tramo);
    vals->rows=rows_salida;
    vals->cols=col;
    return salida;
  } else{
      if(vals->ofd!=NULL){
        pgm_writepgm(vals->ofd,rotada,cols+Extrcols,rows+Extrrows,imaxval,vals->out_ascii);
        if(vals->ofd!=stdout)
          fclose(vals->ofd);
      }
      pgm_freearray(frase,rows);
      free(inicio_tramo); free(fin_tramo);
      vals->rows=rows+Extrrows;
      vals->cols=cols+Extrcols;
      return rotada;
    }
     // pgm_writepgm(ofd,img_crop,col,row,imaxval,out_ascii);

  //pgm_freearray(img_crop,row);
 }
