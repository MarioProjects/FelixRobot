/*
 *  main.cc
 *
 *  Created on: Feb 22, 2007
 *  Authors: atoselli@iti.upv.es and moises@iti.upv.es
 */

#include <unistd.h>
#include <version.h>
#include <iostream>
#include <istream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <cstring>
#include "online.h"
#include "read.h"

using namespace std;

void usage (char *f) {
    cerr << endl;
    cerr << "Usage: " << f << " -s fileSegmentation [<fileMoto> || (def. stdin)] [-V]" << endl;
    cerr << endl;
    return;
}

void version() {
    cerr << IATROS_HTR_ONLINE_PROJECT_STRING"\n"IATROS_HTR_ONLINE_BUILD_INFO;
}


int main(int argc, char ** argv) {
    
    char *prog;
    if ((prog=rindex(argv[0],'/'))) prog+=1;
    else prog=argv[0];

    bool OutFile=false;
    string filename,fileSeg;
    int opt;
    
    while ((opt=getopt(argc,argv,"os:Vh")) != -1)
	switch (opt) {
	    case 'o': OutFile=true; break;
            case 's': fileSeg=optarg; break;
      case 'V': version(); return 1; break;
	    default:  cerr << "ERR: invalid option " << char(opt) << endl;
	    case 'h': usage(prog); return 1;
	};
    
    if (optind<argc) filename.append(argv[optind]);
    ifstream *fd=NULL, aux_fd, fd_seg;
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
//	(*mostres).print_moto();
    } 

    fd_seg.open(fileSeg.c_str());
    if (!fd_seg){
	cerr << "ERR: file \"" << fileSeg.c_str();
	    cerr << "\" can't be opened\n" << endl;
	    return(-1);
    }
    string kk;
    int seg;
    stack<int> segmentacio;
    fd_seg >> kk;
    while (fd_seg >> seg)
	segmentacio.push(seg);
   
    (*mostres). print_moto_seg(filename,segmentacio);

    sprintf(fname,"%s_%d.dat",filename.c_str(),++cont_mostres);   
    
    ostream *fdo;
    if (OutFile) 
	fdo=new ofstream(fname);
     else 
	fdo=&cout;

     //Eixida    
    if (OutFile) 
	delete fdo;
    
    //Libera esto en cada iteración del bucle de sentecias
    delete mostres;
    //delete sent_rot;
    
    // Lineración final de memoria
    delete [] fname;
    
    if (fd!=NULL) fd->close();
    return 0;
}

