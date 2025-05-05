#define MAIN
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include <math.h>
#include <iostream>

GLfloat t;

mat4 projectionMatrix;

Model *octagon, *octagon2;
mat4 octagonPos = T(0.0f, 0.0f, 0.0f);
mat4 octagonScale = S(1.0f, 1.0f, 1.0f);
mat4 octagonPos2 = T(0.0f, 0.0f, 10.0f);
mat4 octagonScale2 = S(2.0f, 2.0f, 2.0f);

mat4 octagonTrans{octagonPos * octagonScale};
mat4 octagonTrans2{octagonPos2 * octagonScale2};

vec3 *vertexArray;
Model *GenerateTerrain(TextureData *tex)
{
    int vertexCount = tex->width * tex->height;
    int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
    unsigned int x, z;

    vertexArray = (vec3 *)malloc(sizeof(GLfloat) * 3 * vertexCount);
    vec3 *normalArray = (vec3 *)malloc(sizeof(GLfloat) * 3 * vertexCount);
    vec2 *texCoordArray = (vec2 *)malloc(sizeof(GLfloat) * 2 * vertexCount);
    GLuint *indexArray = (GLuint *)malloc(sizeof(GLuint) * triangleCount * 3);

    printf("bpp %d\n", tex->bpp);
    for (x = 0; x < tex->width; x++)
        for (z = 0; z < tex->height; z++)
        {
            // Vertex array. You need to scale this properly
            vertexArray[(x + z * tex->width)].x = x / 1.0;
            vertexArray[(x + z * tex->width)].y = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / 10.0;
            vertexArray[(x + z * tex->width)].z = z / 1.0;
            // Normal vectors. You need to calculate these.
            normalArray[(x + z * tex->width)].x = 0.0;
            normalArray[(x + z * tex->width)].y = 1.0;
            normalArray[(x + z * tex->width)].z = 0.0;

            //  Texture coordinates. You may want to scale them.
            texCoordArray[(x + z * tex->width)].x = x; // (float)x / tex->width;
            texCoordArray[(x + z * tex->width)].y = z; // (float)z / tex->height;
        }
    for (x = 0; x < tex->width - 1; x++)
        for (z = 0; z < tex->height - 1; z++)
        {
            // Triangle 1
            indexArray[(x + z * (tex->width - 1)) * 6 + 0] = x + z * tex->width;
            indexArray[(x + z * (tex->width - 1)) * 6 + 1] = x + (z + 1) * tex->width;
            indexArray[(x + z * (tex->width - 1)) * 6 + 2] = x + 1 + z * tex->width;
            // Triangle 2
            indexArray[(x + z * (tex->width - 1)) * 6 + 3] = x + 1 + z * tex->width;
            indexArray[(x + z * (tex->width - 1)) * 6 + 4] = x + (z + 1) * tex->width;
            indexArray[(x + z * (tex->width - 1)) * 6 + 5] = x + 1 + (z + 1) * tex->width;
        }

    for (x = 1; x < tex->width - 1; x++)
    {
        for (z = 1; z < tex->height - 1; z++)
        {
            // Vertex 1
            vec3 p1{vertexArray[((x - 1) + (z - 1) * tex->width)]};
            // Vertex 2
            vec3 p2{vertexArray[(x + (z + 1) * tex->width)]};
            // Vertex 3
            vec3 p3{vertexArray[((x + 1) + z * tex->width)]};

            vec3 v1{p3 - p1};
            vec3 v2{p2 - p1};
            vec3 genNormal{normalize(cross(v1, v2))}; // rimligt
            genNormal.y = abs(genNormal.y);
            normalArray[(x + z * tex->width)] = genNormal;
        }
    }

    // End of terrain generation

    // Create Model and upload to GPU:

    Model *model = LoadDataToModel(
        vertexArray,
        normalArray,
        texCoordArray,
        NULL,
        indexArray,
        vertexCount,
        triangleCount * 3);

    return model;
}

// vertex array object
Model *m, *m2, *tm;
// Reference to shader program
GLuint program;
GLuint tex1, tex2, stex;
TextureData ttex; // terrain

// Movement and look
double movementSpeed = 0.5;
vec3 cameraDirection;
vec3 normalCameraDirection(0.0f, 0.0f, -1.0f);
vec3 cameraPosition(50.0f, 20.f, -100.0f);
mat4 worldToView = lookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
                          cameraPosition.x + normalCameraDirection.x, cameraPosition.y + normalCameraDirection.y, cameraPosition.z + normalCameraDirection.z,
                          0, 1, 0);
;

vec2 direction(0.0f, 0.0f);
void mouseMoved(int x, int y)
{
    float change_x = (x - 300) * 0.000025f;
    float change_y = (300 - y) * 0.000025f;
    direction.x += change_x;
    direction.y += change_y;
    cameraDirection.x = cos((180.0 / M_PI) * direction.x);
    cameraDirection.y = sin((180.0 / M_PI) * direction.y);
    cameraDirection.z = sin((180.0 / M_PI) * direction.x);
    normalCameraDirection = normalize(cameraDirection);
    // std::cout << normalCameraDirection.x << " " << normalCameraDirection.y << " " << normalCameraDirection.z << std::endl;
    // printf("%d %d\n", x, y);
    glutWarpPointer(300, 300);
}

void keyboardPress()
{

    if (glutKeyIsDown('d'))
        cameraPosition += movementSpeed * normalize(cross(normalCameraDirection, vec3(0.0f, 0.1f, 0.0f)));
    else if (glutKeyIsDown('a'))
        cameraPosition -= movementSpeed * normalize(cross(normalCameraDirection, vec3(0.0f, 0.1f, 0.0f)));
    else if (glutKeyIsDown('w'))
        cameraPosition += movementSpeed * normalCameraDirection;
    else if (glutKeyIsDown('s'))
        cameraPosition -= movementSpeed * normalCameraDirection;
}

void init(void)
{

    unsigned int vertexBufferObjID;
    // GL inits
    glClearColor(0.4, 0.6, 0.9, 0);
    glutRepeatingTimer(16.67);

    octagon = LoadModel("../resources/groundsphere.obj");
    octagon2 = LoadModel("../resources/groundsphere.obj");

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glutPassiveMotionFunc(mouseMoved);
    glutHideCursor();
    printError("GL inits");

    projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 1000.0);

    // Load and compile shader
    program = loadShaders("terrain3.vert", "terrain3.frag");
    glUseProgram(program);
    printError("init shader");

    // load and bind grass texture (tex1)
    LoadTGATextureSimple("../resources/grass.tga", &tex1);
    LoadTGATextureSimple("../resources/snow_ice.tga", &stex);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex1);

    // load and bind snow texture (stex)

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, stex);

    // glActiveTexture(GL_TEXTURE0);

    glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);     // Texture unit 0
    glUniform1i(glGetUniformLocation(program, "snowTex"), 1); // Texture unit 1

    glGenBuffers(1, &vertexBufferObjID);

    // glUniform3fv(glGetUniformLocation(program, "inHeight"),1, vertexArray);

    // LoadTGATextureSimple("maskros512.tga", &tex1);
    // Load terrain data

    LoadTGATextureData("../resources/fft-terrain1024c.tga", &ttex);
    tm = GenerateTerrain(&ttex);
    printError("init terrain");

    printf("Note: The call to DrawModel will report warnings about inNormal not existing. This is because inNormal is not used in the shader yet so it is optimized away.\n");

    /*vec3 lightSourceColor = vec3(1.0f, 0.0f, 0.0f);
    vec3 lightSourceDirection = vec3(0.7f, 0.0f, 0.7f);
    glUniform3fv(glGetUniformLocation(program, "lightSourceDirection"), 1, (GLfloat *)&lightSourceDirection);
    glUniform3fv(glGetUniformLocation(program, "lightSourceColor"), 1, (GLfloat *)&lightSourceColor);*/

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
    glBufferData(GL_ARRAY_BUFFER, (ttex.width * ttex.height) * sizeof(vec3), vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(glGetAttribLocation(program, "inHeight"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glGetAttribLocation(program, "inHeight"));
}
GLboolean stop = false;
void display(void)
{

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

    mat4 total, modelToWorld;

    printError("pre display");

    glUseProgram(program);

    // Build matrix

    // vec3 cam = vec3(0, 5, 8);
    // vec3 lookAtPoint = vec3(2, 0, 2);
    /*worldToView = lookAt(cam.x, cam.y, cam.z,
                         lookAtPoint.x, lookAtPoint.y, lookAtPoint.z,
                         0.0, 1.0, 0.0);*/

    worldToView = lookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
                         cameraPosition.x + normalCameraDirection.x, cameraPosition.y + normalCameraDirection.y, cameraPosition.z + normalCameraDirection.z,
                         0, 1, 0);

    modelToWorld = IdentityMatrix();
    total = worldToView * modelToWorld;
    glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);

    // glBindTexture(GL_TEXTURE_2D, tex1); // Bind Our Texture tex1
    glUniformMatrix4fv(glGetUniformLocation(program, "trans"), 1, GL_TRUE, T(0, 0, 0).m);
    DrawModel(tm, program, "inPosition", "inNormal", "inTexCoord");

    printError("display 2");
    float velocity;
    if (!stop)
    {
        velocity = (float)t * 0.005;
    }
    else
    {
        velocity = 0;
    }
    float octxf = 0.0f;
    float octzf = 100.0f;
    int octx, octz;
    octxf = velocity;
    octzf = velocity;
    octx = (int)octxf;
    octz = (int)octzf;

    TextureData *octTex = &ttex;
    float octMax = 0.0;
    if (vertexArray[(octx + octz * octTex->width)].y > octMax)
    {
        octMax = vertexArray[(octx + octz * octTex->width)].y;
    };
    if (vertexArray[(octx + (octz + 1) * octTex->width)].y > octMax)
    {
        octMax = vertexArray[(octx + (octz + 1) * octTex->width)].y;
    };
    if (vertexArray[(octx + 1 + octz * octTex->width)].y > octMax)
    {
        octMax = vertexArray[(octx + 1 + octz * octTex->width)].y;
    };
    if (vertexArray[(octx + 1 + (octz + 1) * octTex->width)].y > octMax)
    {
        octMax = vertexArray[(octx + 1 + (octz + 1) * octTex->width)].y;
    };

    octagonPos = T(octxf, octMax, octzf);
    octagonTrans = mat4(octagonPos * octagonScale);

    float oct2xf = 100.0f;
    float oct2zf = 0.0f;
    int oct2x, oct2z;
    oct2xf -= velocity;
    oct2zf += velocity;
    oct2x = (int)oct2xf;
    oct2z = (int)oct2zf;

    float oct2Max = 0.0;
    if (vertexArray[(oct2x + oct2z * octTex->width)].y > oct2Max)
    {
        oct2Max = vertexArray[(oct2x + oct2z * octTex->width)].y;
    };
    if (vertexArray[(oct2x + (oct2z + 1) * octTex->width)].y > oct2Max)
    {
        oct2Max = vertexArray[(oct2x + (oct2z + 1) * octTex->width)].y;
    };
    if (vertexArray[(oct2x + 1 + oct2z * octTex->width)].y > oct2Max)
    {
        oct2Max = vertexArray[(oct2x + 1 + oct2z * octTex->width)].y;
    };
    if (vertexArray[(oct2x + 1 + (oct2z + 1) * octTex->width)].y > oct2Max)
    {
        oct2Max = vertexArray[(oct2x + 1 + (oct2z + 1) * octTex->width)].y;
    };

    // collisions:

    vec3 oct1pos = vec3(octxf, octMax, octzf);
    vec3 oct2pos = vec3(oct2xf, oct2Max, oct2zf);
    vec3 diff = oct1pos - oct2pos;
    float distSquared = dot(diff, diff);
    printf("%f\n", velocity);
    if (distSquared < 9)
    {
        stop = true;
    }

    octagonPos = T(oct1pos.x, oct1pos.y, oct1pos.z);
    octagonTrans = mat4(octagonPos * octagonScale);
    glUniformMatrix4fv(glGetUniformLocation(program, "trans"), 1, GL_TRUE, octagonTrans.m);
    DrawModel(octagon, program, "inPosition", NULL, NULL);

    octagonPos2 = T(oct2pos.x, oct2pos.y, oct2pos.z);
    octagonTrans2 = mat4(octagonPos2 * octagonScale2);
    glUniformMatrix4fv(glGetUniformLocation(program, "trans"), 1, GL_TRUE, octagonTrans2.m);
    DrawModel(octagon2, program, "inPosition", NULL, NULL);

    keyboardPress();
    glutWarpPointer(300, 300);

    glutSwapBuffers();
}

void mouse(int x, int y)
{
    // This function is included in case you want some hints about using passive mouse movement.
    // Uncomment to see mouse coordinates:
    // printf("%d %d\n", x, y);
}
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion(3, 2);
    glutInitWindowSize(1920, 1080);
    // glutToggleFullScreen();
    glutCreateWindow("TSBK07 Lab 4.5");
    glutDisplayFunc(display);
    init();
    glutMainLoop();
    return 0;
}
