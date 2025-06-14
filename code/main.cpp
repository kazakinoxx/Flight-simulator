#define MAIN
#include "helperFun.h"
#include <stdio.h>
#include <stdlib.h>
float cockpitVerts[] = {
   
    -1.0f, -1.0f, 0, 0.0f, 0.0f, 
    1.0f, -1.0f, 0, 1.0f, 0.0f,  
    1.0f, -0.3f, 0, 1.0f, 1.0f,  

    -1.0f, -1.0f, 0, 0.0f, 0.0f, 
    1.0f, -0.3f, 0, 1.0f, 1.0f,  
    -1.0f, -0.3f, 0, 0.0f, 1.0f  
};


void init(void)
{

    // GL inits
    glClearColor(0.4, 0.6, 0.9, 0);
    glutRepeatingTimer(16.67);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glutHideCursor();
    printError("GL inits");

    // Load and compile shader
    overlayprogram = loadShaders("over.vert", "over.frag");
    program = loadShaders("terrain3.vert", "terrain3.frag");
    glUseProgram(program);

    printError("init shader");

    skybox = LoadModel("../resources/skybox-full-tweaked.obj");

    // load and bind texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    LoadTGATextureSimple("../resources/cliff_pink_20.tga", &tex1);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &stex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, stex);

    glUniform1i(glGetUniformLocation(program, "snowTex"), 1);

    GLint width = 512;
    GLint height = 512;
    int numLayer = 7;

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, width, height, numLayer);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    multiBind(numLayer,width,height);


    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &tex2);
    glBindTexture(GL_TEXTURE_2D, tex2);
    LoadTGATextureSimple("../resources/ter_sand1.tga", &tex2);
    glUniform1i(glGetUniformLocation(program, "gtex"), 2);

    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &SkyTex);
    glBindTexture(GL_TEXTURE_2D, SkyTex);
    LoadTGATextureSimple("../resources/cubemap.tga", &SkyTex);

    glUniform1i(glGetUniformLocation(program, "sky"), 3);


    // COCKPIT

    glGenVertexArrays(1, &cockpitVAO);
    glGenBuffers(1, &cockpitVBO);

    glBindVertexArray(cockpitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cockpitVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cockpitVerts), cockpitVerts, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // TexCoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE4);
    glGenTextures(1, &cockpitTex);
    glBindTexture(GL_TEXTURE_2D, cockpitTex);
    LoadTGATextureSimple("../resources/cockpit.tga", &cockpitTex);
    glUniform1i(glGetUniformLocation(overlayprogram, "cock"), 4);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);

    // Load terrain data

    LoadTGATextureData("../resources/fft-terrain1024c.tga", &ttex);
    tm = GenerateTerrain(&ttex);

    printError("init terrain");

    glGenBuffers(1, &vertexBufferObjID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
    glBufferData(GL_ARRAY_BUFFER, (ttex.width * ttex.height) * sizeof(vec3), vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(glGetAttribLocation(program, "inHeight"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glGetAttribLocation(program, "inHeight"));

    // locations needed

    mdlMxloc = glGetUniformLocation(program, "mdlMatrix");
    transMxLoc = glGetUniformLocation(program, "trans");
    
}

void display(void)
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    t = (GLfloat)glutGet(GLUT_ELAPSED_TIME) / 100;

    int layer = (int)(t / 50) % 6;
    glUniform1i(glGetUniformLocation(program, "layer"), layer);

    glUseProgram(program);

    keyboardPress();
    updatePhysics();
    mat4 worldToView = lookAt(cameraPosition, cameraTarget, up);

    mat4 modelToWorld = IdentityMatrix();
    mat4 total = worldToView * modelToWorld;
    glUniformMatrix4fv(mdlMxloc, 1, GL_TRUE, total.m);

    // skybox
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    mat4 skyModel = T(cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniformMatrix4fv(transMxLoc, 1, GL_TRUE, skyModel.m);
    glUniform1i(glGetUniformLocation(program, "texUnit"), 3);
    DrawModel(skybox, program, "inPosition", "inNormal", "inTexCoord");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glUniform1i(glGetUniformLocation(program, "texUnit"), 0);

    //terrain
    multiGen();


    // cockpit
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(overlayprogram);
    glDisable(GL_DEPTH_TEST);

    mat4 ort = ortho(-1, 1, -1, 1, -1, 1);
    glUniformMatrix4fv(glGetUniformLocation(overlayprogram, "proj"), 1, GL_TRUE, ort.m);

    
    glBindTexture(GL_TEXTURE_2D, cockpitTex);
    glUniform1i(glGetUniformLocation(overlayprogram, "tex"), 4);

    glBindVertexArray(cockpitVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    //finishing the frame

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glUseProgram(program);
    glutSwapBuffers();
    glutSwapBuffers();
    printDebugInfo();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion(3, 2);
    // glutInitWindowSize(1920, 1080);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Flight simulator");
    glutDisplayFunc(display);
    init();
    glutMainLoop();
    return 0;
}
