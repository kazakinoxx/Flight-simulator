#define MAIN
#include "helperFun.h"

unsigned int vertexBufferObjID;
GLint mdlMxloc,transMxLoc;


void init(void)
{

    
    // GL inits
    glClearColor(0.4, 0.6, 0.9, 0);
    glutRepeatingTimer(16.67);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glutPassiveMotionFunc(mouseMoved);
    glutHideCursor();
    printError("GL inits");

   


    // Load and compile shader
    program = loadShaders("terrain3.vert", "terrain3.frag");
    glUseProgram(program);
    printError("init shader");

    // load and bind texture 
    LoadTGATextureSimple("../resources/grass.tga", &tex1);
    LoadTGATextureSimple("../resources/snow_ice.tga", &stex);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, stex);


    glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);     // Texture unit 0
    glUniform1i(glGetUniformLocation(program, "snowTex"), 1); // Texture unit 1


    // Load terrain data

    LoadTGATextureData("../resources/fft-terrain1024c.tga", &ttex);
    tm = GenerateTerrain(&ttex);
   
    printError("init terrain");

    glGenBuffers(1, &vertexBufferObjID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
    glBufferData(GL_ARRAY_BUFFER, (ttex.width * ttex.height) * sizeof(vec3), vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(glGetAttribLocation(program, "inHeight"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glGetAttribLocation(program, "inHeight"));

    //locations needed


    mdlMxloc = glGetUniformLocation(program, "mdlMatrix");
    transMxLoc = glGetUniformLocation(program, "trans") ;
}



void display(void)
{
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

    printError("pre display");

    

    mat4 worldToView = lookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
                         cameraPosition.x + normalCameraDirection.x, cameraPosition.y + normalCameraDirection.y, cameraPosition.z + normalCameraDirection.z,
                         0, 1, 0);

    mat4 modelToWorld = IdentityMatrix();
    mat4 total = worldToView * modelToWorld;


    glUniformMatrix4fv(mdlMxloc, 1, GL_TRUE, total.m);
    glUniformMatrix4fv(transMxLoc, 1, GL_TRUE, T(0, 0, 0).m);


    DrawModel(tm, program, "inPosition", "inNormal", "inTexCoord");


    printError("display 2");
    
    keyboardPress();
    glutWarpPointer(300, 300);
    glutSwapBuffers();
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
