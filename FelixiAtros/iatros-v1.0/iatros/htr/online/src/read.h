/*
 *  read.h
 *
 *  Created on: Feb 12, 2006
 *  Author: moises@iti.upv.es
 */

#ifndef READ_H
#define READ_H

#include <istream>
#include <string>
#include <vector>
#include "online.h"

#define MAX_LIN 250

using namespace std;

int read_file_moto(istream &fd, sentence ** S, string filename);

#endif
 
