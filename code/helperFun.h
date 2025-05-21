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
extern TextureData ttex;
extern double movementSpeed;
extern vec3 cameraDirection;
extern vec3 normalCameraDirection;
extern vec3 cameraPosition;
extern mat4 orientation;

extern vec2 direction;

extern float throttle;
extern float throttleFactor;
extern float speed;
extern float sidewaysSpeed;
extern float airDragFactor;
extern float verticalDragFactor;
extern int deltaRoll;
extern int deltaPitch;
extern int roll;
extern int pitch;
extern bool upsideDownRoll;
extern bool upsideDownPitch;

Model *GenerateTerrain(TextureData *tex);
void keyboardPress();
mat4 rotationMatrix(vec3 axis, float angle);
// void mouseMoved(int x, int y);
