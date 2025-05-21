# set this variable to the director in which you saved the common files
commondir = ../../common/

all : corridor corridor-plus-portal

corridor : corridor.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o corridor -I$(commondir) -I$(commondir)Linux -DGL_GLEXT_PROTOTYPES corridor.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

corridor-plus-portal : corridor-plus-portal.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o corridor-plus-portal -I$(commondir) -I$(commondir)Linux -DGL_GLEXT_PROTOTYPES corridor-plus-portal.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

clean :
	rm corridor
	rm  corridor-plus-portal

