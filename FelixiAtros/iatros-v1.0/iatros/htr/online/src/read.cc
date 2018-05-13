/*
 *  read.cc
 *
 *  Created on: Feb 12, 2006
 *  Author: moises@iti.upv.es
 */


#include <string>
#include <cstring>
#include "read.h"


int read_file_moto(istream &fd, sentence ** S, string filename) {
  
  char linea[MAX_LIN];
  static int line_counter=0;

  while (fd.getline(linea,MAX_LIN)) {
    line_counter++;
    if (linea[0]!='#' && linea[0]!=' ' && strlen(linea)>0) break;
    //if (linea[0]!=' ' && strlen(linea)>0) break;
  }
  if (fd.eof()) return false;
  //if (!isalpha(linea[0])) {
  if (!isgraph(linea[0])) {
      cerr << "ERR 2: incorrect file format. " << filename.c_str()<< "  Line " << line_counter << endl;
    return false;
  }
  // nueva sentencia
  int n_strokes;
  fd >> n_strokes;
  line_counter++;
  sentence *sent=new sentence(linea,n_strokes);
  // leemos las trazas
  for (int s=0; s<n_strokes; s++) {		
    int num_points_stroke;
    line_counter++;
    fd >> num_points_stroke;
    if (!fd.good()) {
      cerr << "ERR 1: incorrect file format. "  << filename.c_str()<<"  Line: " << line_counter << endl;
      return false;
    }
      
    int is_pen_down;
    line_counter++;
    fd >> is_pen_down;
    if (!fd.good()) {
      cerr << "ERR 1: incorrect file format. " << filename.c_str()<<"  Line: " << line_counter << endl;
      return false;
    }
      
    stroke st(num_points_stroke,is_pen_down);
    // leemos los puntos de cada traza
    for (int i=0; i<num_points_stroke; i++) {
      int x, y;
      line_counter++;
      fd >> x >> y;
      if (!fd.good()) {
	cerr << "ERR 1: incorrect file format. " << filename.c_str()<< "  Line: " << line_counter << endl;
        return false;
      }
      Point p(x,y);
      st.points.push_back(p);
    }
    // guardamos cada traza en la sentencia
    if (st.points.size()>0) (*sent).strokes.push_back(st);
    else if (n_strokes>0) (*sent).n_strokes--;
  }
  *S=sent;
    
  // para el último salto de carro
  fd.getline(linea,MAX_LIN);

  return true;
}

