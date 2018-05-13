#ifndef ONLINE_H
#define ONLINE_H

#include <math.h>
#include <values.h>
#include <iostream>
#include <string>
#include <vector>
#include <stack>

using namespace std;

//Clase para el Punto Real
class PointR {
public:
    float x, y;
private:
    // Para marcar el punto en caso de ser el último de cada trazo
    // Se utiliza en el cálculo de los bitmap-featureY
    bool point_pu;
public:
    PointR(float _x, float _y): x(_x), y(_y), point_pu(false) {}
    PointR & operator= (const PointR & p) {
      x=p.x; y=p.y;
      point_pu=p.point_pu;
      return *this;
    }
    bool operator ==(const PointR & p) const {
      return p.x==x && p.y==y;
    }
    bool operator !=(const PointR & p) const {
      return p.x!=x || p.y!=y;
    }
    void print() {
      cout << x << " " << y << endl;
    }
    void setpu() {
      point_pu=1;
    }
    bool getpu() {
      return point_pu;
    }
};

//Clase para el Punto
class Point {
public:
    int x, y;
private:
    // Para marcar el punto en caso de ser el último de cada trazo
    // Se utiliza en el cálculo de los bitmap-feature
    bool point_pu;
public:
    Point(int _x, int _y): x(_x), y(_y), point_pu(false) {}
    Point & operator= (const Point & p) {
      x=p.x; y=p.y;
      point_pu=p.point_pu;
      return *this;
    }
    bool operator == (const Point & p) const {
      return p.x==x && p.y==y;
    }
    bool operator !=(const Point & p) const {
      return p.x!=x || p.y!=y;
    }
    void print(ostream & fd) {
      fd << x << " " << y << endl;
    }
    void setpu() {
      point_pu=1;
    }
    bool getpu() {
      return point_pu;
    }
};

class stroke {
  public:
    int n_points;
    bool pen_down;
    bool is_hat;
    vector<Point> points;
    
    stroke(int n_p=0, bool pen_d=0, bool is_ht=0);
  
    void print();
    void print_moto(ostream & fd);

    float calcular_y_media_stroke();  
    float calcula_long_stroke();

    int F_XMIN();
    int F_XMAX();
    int F_XMED();
};

class sentence {
  public:
    string transcrip;
    int n_strokes;
    vector<stroke> strokes;
    
    sentence(string w,int n_s);

    void print(bool print_pen_up=false);
    void print_moto(bool print_pen_up=false);
    void print_moto_seg(string & filename, stack<int> & segmentacio);
    sentence * anula_rep_points();
    sentence * suaviza_traza(int cont_size=2);
    sentence * normaliza_traza(float L=0.0);  // Espaciado entre puntos
    sentence * normaliza_traza(int M=0);      // Numero de puntos
    sentence * interpol_strokes_pu(); 

    vector<Point> * minimos_locales(float & mean, float & desv);
    float calcuar_media_y();
    
    sentence * rotate(float angle);
    
    void marca_back(float thres);

    float MVPV(float thresh_slt,double &ymin);
    sentence * tr_shift(float angle, double ref);
  
  private:
    int ** get_projections(vector<Point> &points, double xmin, double xmax, double ymin, double ymax);
    inline float var_VProjection(int *VPr, int cols);
};


#endif
 
