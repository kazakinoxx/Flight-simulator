# set this variable to the directory in which you saved the common files
commondir = ../common/

all : main

main : main.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o main -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES main.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lX11 -lGL -lm

clean :
	rm main
