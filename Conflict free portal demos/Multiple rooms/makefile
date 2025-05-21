# set this variable to the director in which you saved the common files
commondir = ../../common/

all : multiple-two multiple-four multiple-auto

multiple-two : multiple-two.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o multiple-two -I$(commondir) -I$(commondir)Linux -DGL_GLEXT_PROTOTYPES multiple-two.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

multiple-four : multiple-four.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o multiple-four -I$(commondir) -I$(commondir)Linux -DGL_GLEXT_PROTOTYPES multiple-four.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

multiple-auto : multiple-auto.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o multiple-auto -I$(commondir) -I$(commondir)Linux -DGL_GLEXT_PROTOTYPES multiple-auto.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

clean :
	rm multiple-two
	rm  multiple-four
	rm  multiple-auto

