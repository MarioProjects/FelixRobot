BINARIES=	featExtOnLine

CC	=	g++

#STATIC  =	-static
#GPROF	=	-pg
#DEBUG	=	-g $(GPROF)
#LIBS	=	-L/usr/lib64/ -lfftw3 -lm -m64
LIBS	=	-L/usr/lib -lfftw3 -lm 

ifndef DEBUG
CFLAGS	=	-Wall -Wno-sign-compare -O4 $(STATIC)
else
CFLAGS	=	-Wall -Wno-sign-compare $(DEBUG)
endif

OBJ	=	online.o read.o features.o main.o


all: $(BINARIES)

$(BINARIES): $(OBJ)
	$(CC) $(CFLAGS) -o $(BINARIES) *.o $(LIBS)
ifndef DEBUG	
	strip $(BINARIES)
endif

main.o: main.cc online.h read.h features.h
	$(CC) $(CFLAGS) -c $< -o $@

features.o: features.cc features.h online.h
	$(CC) $(CFLAGS) -c $< -o $@

read.o: read.cc read.h online.h
	$(CC) $(CFLAGS) -c $< -o $@

online.o: online.cc online.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o *~ gmon.out $(BINARIES)

