# set this variable to the director in which you saved the common files
commondir = ../../common/

all : wall wall-plus-light wall-plus-portal

wall : wall.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o wall -I$(commondir) -I$(commondir)Linux -DGL_GLEXT_PROTOTYPES wall.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

wall-plus-light : wall-plus-light.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o wall-plus-light -I$(commondir) -I$(commondir)Linux -DGL_GLEXT_PROTOTYPES wall-plus-light.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

wall-plus-portal : wall-plus-portal.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o wall-plus-portal -I$(commondir) -I$(commondir)Linux -DGL_GLEXT_PROTOTYPES wall-plus-portal.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

clean :
	rm wall
	rm  wall-plus-light
	rm  wall-plus-portal

