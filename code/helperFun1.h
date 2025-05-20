#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include <math.h>
#include <iostream>


extern GLfloat t;

extern GLuint program;
extern mat4 projectionMatrix;
extern vec3 *vertexArray;
extern Model *tm;
extern GLuint tex1, tex2, stex;
extern TextureData stex2;
extern TextureData ttex;
extern double movementSpeed ;
extern vec3 cameraDirection;
extern vec3 normalCameraDirection;
extern vec3 cameraPosition;

extern vec2 direction;



Model *GenerateTerrain(TextureData *tex);
void keyboardPress();
void mouseMoved(int x, int y);

