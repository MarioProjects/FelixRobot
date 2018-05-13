#!/bin/bash

# Copyright (C) 1997 by Pattern Recognition and Human Language
# Technology Group, Technological Institute of Computer Science,
# Valencia University of Technology, Valencia (Spain).
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.  This software is provided "as is" without express or
# implied warranty.

export LC_NUMERIC=C

PROG=${0##*/}
if [ $# -ne 1 -a $# -ne 4 ]; then
  echo "Usage: $PROG <features-file> [nlU nlC nlL]";
  exit;
fi

FILE=$1
if [ $# -eq 4 ]; then
  NLU=$2
  NLC=$3
  NLL=$4
  echo $NLU $NLC $NLL
fi

TMP=./$$.aux
trap "rm $TMP* 2>/dev/null" EXIT

cat $FILE > $TMP
#sed -r 0,/^[Dd]ata$/d $FILE > $TMP
maxval=255
Wide=$(awk 'END{print NR}' $TMP)
Height=$(awk 'END{print int(NF/3)}' $TMP)
echo $Wide $Height $maxval

if [ $# -eq 4 ]; then
    sum=$[NLU+NLC+NLL]
    if [ $sum -ne $Height ]; then
	echo "The SUM nlU+nlC+nlL=$sum, isn't equal to the image height=$Height"
	exit
    fi
    awk -v u=$NLU -v c=$NLC -v l=$NLL \
    '{ 
          for (k=0; k<3; k++) {
	     b=1;
             for (i=b+k*u; i<=u+b+k*u-1; i++) printf $i" "; b+=u*3;
	     for (i=b+k*c; i<=c+b+k*c-1; i++) printf $i" "; b+=c*3;
	     for (i=b+k*l; i<=l+b+k*l-1; i++) printf $i" ";
          }
	  print ""
     }' $TMP > aux
    mv aux $TMP
fi

awk -v h=$Height -v w=$Wide -v mx=$maxval '
BEGIN{printf("P2\n%d %d\n%d\n",w,h,mx)}
{for (i=1;i<=h;i++) C[i,NR]=$i}
END{for (i=1;i<=h;i++) {for (j=1;j<=w;j++)
printf("%d ",int(mx*((100-C[i,j])/100)));
printf("\n")}}' $TMP | 
#xv -name "Nivel de Gris" -geometry "+0+0" -nolimits -raw - & 
xv -name "Nivel de Gris" -geometry "+0+0" -maxpect -raw - & 

awk -v h=$Height -v w=$Wide -v mx=$maxval -v pi=3.141516 '
BEGIN{printf("P2\n%d %d\n%d\n",w,h,mx)}
{for (i=1;i<=h;i++) C[i,NR]=$(i+h)}
END{for (i=1;i<=h;i++) {for (j=1;j<=w;j++)
printf("%d ",int(mx*((pi/2+C[i,j])/pi)));
printf("\n")}}' $TMP |
#xv -name "Der. Horiz." -geometry "+0+150" -norm -nolimits -raw - & 
xv -name "Der. Horiz." -geometry "+0+150" -norm -maxpect -raw - & 

awk -v h=$Height -v w=$Wide -v mx=$maxval -v pi=3.141516 '
BEGIN{printf("P2\n%d %d\n%d\n",w,h,mx)}
{for (i=1;i<=h;i++) C[i,NR]=$(i+2*h)}
END{for (i=1;i<=h;i++) {for (j=1;j<=w;j++)
printf("%d ",int(mx*((pi/2+C[i,j])/pi)));
printf("\n")}}' $TMP |
#xv -name "Der. Vertic." -geometry "+0+300" -norm -nolimits -raw - &
xv -name "Der. Vertic." -geometry "+0+300" -norm -maxpect -raw - &

sleep 1
