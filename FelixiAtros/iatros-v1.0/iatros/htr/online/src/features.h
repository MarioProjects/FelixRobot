/*
 *  features.h
 *
 *  Created on: Feb 12, 2006
 *  Authors: atoselli@iti.upv.es and moises@iti.upv.es
 */

#ifndef FEATURES_H
#define FEATURES_H

#include <cmath>
#include <fftw3.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <values.h>
#include "online.h"

#define MAXNUMHATS 200
#define OFFSET_INS 20

using namespace std;

class frame {
  public:
    double x,y,dx,dy,ax,ay,k,ang;
    vector<double> fft_feat;

    void print(ostream & fd);
    int get_fr_dim();
};

class sentenceF {
  public:
    string transcrip;
    int n_frames;
    frame * frames;
    
    sentenceF();
    ~sentenceF();

    bool data_plot(ostream & fd);
    bool print_FFT(ostream & fd, int C=0);
    bool print(ostream & fd);

    void calculate_features(sentence &s, char pre_hat='\0', int width_w=5, int nfft=0);

  private:
    vector<PointR> normalizaAspect(vector<Point> & puntos);
    void calculate_derivatives(vector<PointR> & points, bool norm=true);
    void calculate_kurvature();
    void calculate_FFT_feat(vector<PointR> & pointsN, int width_w, int step=1, int nfftp=0);
};


#endif
