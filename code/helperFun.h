#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include <math.h>
#include <iostream>
#include <vector>

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
extern vec3 up;
extern vec3 cameraTarget;
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

Model *GenerateTerrain(TextureData *tex);
void keyboardPress();
mat4 rotationMatrix(vec3 axis, float angle);
void updatePhysics();


struct InstanceData;

void printDebugInfo();
void multiGen();

void multiBind(int numLayer, int width, int height);

extern std::vector<InstanceData> instances;

extern unsigned int vertexBufferObjID;

extern GLint mdlMxloc, transMxLoc;
extern Model *skybox;
extern Model *cockpitQuad;

extern unsigned int SkyTex;
extern GLuint cockpitTex;
extern GLuint overlayprogram;

extern GLuint cockpitVBO, cockpitVAO;
extern GLuint vao;


extern GLuint instanceVBO;

extern TextureData textures[8];