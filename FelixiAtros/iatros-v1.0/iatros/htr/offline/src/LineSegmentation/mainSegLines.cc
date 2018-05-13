#include "segLinesClass.h"
#include <unistd.h>
#include <string.h>
#include <strings.h>


void usage(char * nomProg) {
  cerr << endl;
  cerr << "Usage: " << nomProg << " [options]" << endl;
  cerr << "      options:" << endl;
  cerr << "          -i <file.pgm>                (by default stdin)" << endl;
  cerr << "          -b [0:100]" << endl;
  cerr << "             Overlapping factor" << endl;
  cerr << "             between lines in %        (by default 0)" << endl;
  // cerr << "          -p [1:255]" << endl;
//  cerr << "             Grey level threshold" << endl;
  cerr << "          -p Performs line segm. with" << endl;
  cerr << "             cleaning and relocating" << endl;
  cerr << "             ascendent strokes using" << endl;
  cerr << "             connected components  " /* (Uses OTSU by setting this to 1)"*/ << endl;
  cerr << "          -l [-100:100]" << endl;
  cerr << "             Valey detection" << endl;
  cerr << "             threshold in %            (by default 0)" << endl;
  cerr << "          -f" << endl;
  cerr << "             Print line segment" << endl;
  cerr << "             information               (by default false)" << endl;
  cerr << "          -d" << endl;
  cerr << "             Page segmentation demo    (by default false)" << endl; 
  cerr << "          -s <file.seg>" << endl;
  cerr << "             Segm. file information    (by default none)" << endl;
//  cerr << "          -n <file.pgm>" << endl;
//  cerr << "             Image with lines;         (by default none)" << endl;
  cerr << "          -x" << endl;
  cerr << "             Look for short lines" << endl;
  cerr << "             between the valleys       (by default false)" << endl;
  cerr << "          -v <#>                       (by default 0)" << endl;
  cerr << "                > 0  --> Horizontal Histogram" << endl;
  cerr << "                > 1  --> ... + Segm. Lines Information" << endl;
  cerr << "                > 2  --> ... + Gray Level Histogram" << endl;
  cerr << "                > 3  --> ... + MSFS information" << endl;
  cerr << "          -h" << endl;
  cerr << "             this help" << endl;
  cerr << "          " << endl;
  cerr << endl;
}


int main(int argc, char ** argv) {

  char *prog, *ifname=NULL, *sfname=NULL, *imgname=NULL;
  if ((prog=rindex(argv[0],'/'))) prog+=1;
  else prog=argv[0];

  bool entrada_estandard=true, seginfile=false;
  bool divline=false, demo=false, segfile=false;
  int option;
  ifstream ifd;
  int overlap=0, valeyDectLevel=0, verbosity=0;

  while ((option=getopt(argc,argv,"i:b:pl:fds:n:xv:h"))!=-1)
    switch (option) {
    case 'i':
      ifname=optarg;
      ifd.open(ifname);
      if (!ifd) {
        cerr << "Error: File \"" << ifname << "\" could not be open."<< endl;
        exit (-1);
      }
      entrada_estandard=false;
      break;
    case 'b':
      overlap=atoi(optarg);
      break;
    case 'p':
      // To mean that we want this option to be active
      //overlap=atoi(optarg); 
      overlap=-1;
      break;
    case 'l':
      valeyDectLevel=atoi(optarg);
      break;
    case 'f':
      seginfile=true;
      break;
    case 'd':
      demo=true;
      break;
    case 's':
      segfile=true;
      sfname=optarg;
      break;
 //    case 'n':
 //    segfile=true;
 //    imgname=optarg;
 //     break;
    case 'x':
      divline=true;
      break;
    case 'v':
      verbosity=atoi(optarg);
      break;
    case 'h':
    default:
      usage(prog);
      exit(1);
    }

  lineSegm image;
  if (entrada_estandard) {
    cerr << "standard imput...\n";
    ifname="stdin";
    image.read(cin);
  } else {
    // Anula el sufijo ".pgm" de la cedena ifname
    ifname[strlen(ifname)-4]='\0';
    image.read(ifd);
  }  

  
  image.verbosity=verbosity;
  
  if (segfile)
    image.fileSegLines(ifname,sfname,imgname,overlap,demo);
  else
    image.makeSegLines(ifname,overlap,valeyDectLevel,divline,demo);
  
  if (seginfile) image.writeSegmInf(ifname);
 
  ifd.close();
  return 0;

}
