#!/bin/bash

export LC_NUMERIC="C"
shopt -s extglob

PROG=${0##*/}

PS=0
PU=0
NUM=0
SUA=0
while [ $# -ge 1 ]; do
  if   [ "${1:0:3}" = "-ps" ]; then PS=1
  elif [ "${1:0:3}" = "-pu" ]; then PU=1
  elif [ "${1:0:2}" = "-n" ]; then shift; NUM=$1
  elif [ "${1:0:2}" = "-s" ]; then shift; SUA=$1
  else break
  fi
  shift
done
if [ $# -ne 1 ]; then
  echo "Usage: $PROG [-ps] [-pu] [-n <numSample>] <file.dat>" 1>&2
  exit -1
fi

DATA=$1

ARCH=/tmp/$PROG.$$
trap "rm ${ARCH}_* ${ARCH}*.lm 2>/dev/null" EXIT


#GRAPHFLAGS="-B -T X --bitmap-size 1200x600 -h .8 -w .8 -F HersheySerif -f .02 -r .1 -u .1 --frame-color 1 -y 600 200 -x 0 1500"
#GRAPHFLAGS="-T X -C --bitmap-size 1200x600 -h .8 -w .8 -F HersheySerif -f .02 --title-font-size 0.05 -r .1 -u .1 -N X -N Y"
GRAPHFLAGS="-T X -C --bitmap-size 600x300 -h .8 -w .8 -F HersheySerif -f .02 --title-font-size 0.05 -r .1 -u .1 -N X -N Y"

gunzip -fc $DATA | dos2unix |
awk -v Name="$ARCH" -v Pu=$PU -v k=$SUA '
  BEGIN{cont=0}
  {
    if (/^[# ]+$/) next; 
    else {
      Et[++cont]=$0;
      xmax[cont]=-100000;
      xmin[cont]=100000;
      ymax[cont]=-100000;
      ymin[cont]=100000;
      getline;
      if (NF==1 && $1~/[0-9]+/) {
        Nt[cont]=$1;
      } else { print "SYNTAX ERROR 1" > "/dev/stderr"; exit; }
    } 

    for (nt=1;nt<=Nt[cont];nt++) {
      getline;
      if (NF==1 && $1~/[0-9]+/) {
        Np[nt,cont]=$1;
        getline;
      } else {print "SYNTAX ERROR 2" > "/dev/stderr"; exit; }
      
      if (NF==1 && ($NF=="1" || $NF=="0")) F[nt,cont]=$1;
      else { print "SYNTAX ERROR 3" > "/dev/stderr"; exit; }
      
      for (np=1;np<=Np[nt,cont];np++) { 
        getline;
        if (NF==2) {
          if (xmax[cont]<$1) xmax[cont]=$1; 
          if (xmin[cont]>$1) xmin[cont]=$1; 
          if (ymax[cont]<$2) ymax[cont]=$2; 
          if (ymin[cont]>$2) ymin[cont]=$2; 
          X[np,nt,cont]=$1; Y[np,nt,cont]=$2;
	} else { print "SYNTAX ERROR 4 > /dev/stderr"; exit; }
      }

    }
  }
  END{
    for (s=1;s<=cont;s++) {
      D=Name"_"s; 
      print "-x "xmin[s],xmax[s]" -y "ymin[s],ymax[s],Et[s] > D".lm"; 
      for (i=1;i<=Nt[s];i++) {
        if  (F[i,s]) {print "#m=1,S=1" > D; z=1}
        else if (Pu) {print "#m=5,S=1" > D; z=1}
        else continue;
	#if (i>1) print X[j-1,i-1,s],Y[j-1,i-1,s] > D;
        #for (j=1;j<=Np[i,s];j++) print X[j,i,s],Y[j,i,s] > D; 
        for (j=1;j<=Np[i,s];j++) {
	  if(z) {x=X[j,i,s]; y=Y[j,i,s]; z=0;}
          x=(x*k+X[j,i,s])/(1+k); y=(y*k+Y[j,i,s])/(1+k); 
	  print x,y > D; 
        }
      }
    }
  }'

for f in ${ARCH}_+([0-9]); do
  if [ $NUM -ne 0 ]; then
     [ ${f/*_} -eq $NUM ] || continue
  fi
  echo Displaying Sample: $f 1>&2
  if [ $PS -eq 0 ]; then
    LIMITS=`awk '{xr=($3-$2)*.05;yr=($6-$5)*.05; \
            print $1,int($2-xr),int($3+xr),$4,int($5-yr),int($6+yr)}' $f".lm"`
    LABEL=`awk '{for (i=7;i<NF;i++) printf("%s ",$i); print $NF}' $f".lm"`
#   awk '{system("i=1; while [ $i -le 1000 ]; do i=$[i+1]; done"); print}' $f |
    awk '{system("usleep 1000"); print}' $f |
    graph $GRAPHFLAGS $LIMITS -L "$LABEL" 
    while [ 1 ]; do pgrep -x graph > /dev/null || break ; sleep 1 ; done
  else
    R=$(awk '{r=($6-$5)/($3-$2); if (r<0) r=-r; printf("%1.1f\n",r)}' $f".lm")
    W=1; H=1
    eval $(echo "scale=1; r=$R; if (r<1) {print \"H=\";r} else {print \"W=\";1/r}" | bc -l)
    #echo $R $W $H 1>&2
    graph -T ps -g 0 -W .005 -w $W -h $H $f
    #graph -T gif -g 0 -W .005 -w $W -h $H $f
  fi
done
