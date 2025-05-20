#define MAIN
#include "helperFun1.h"
#include <random>

unsigned int vertexBufferObjID;
GLint mdlMxloc, transMxLoc;
TextureData textures[8];
vec2 terrains[9];
GLint layerLoc;

void init(void)
{

    // GL inits
    glClearColor(0.4, 0.6, 0.9, 0);
    glutRepeatingTimer(16.67);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    //glutPassiveMotionFunc(mouseMoved);
    glutHideCursor();
    printError("GL inits");

    // Load and compile shader
    program = loadShaders("terrain3.vert", "terrain3.frag");
    glUseProgram(program);
    printError("init shader");

    // load and bind texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);
   
    LoadTGATextureSimple("../resources/cliff_pink_20.tga", &tex1);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &stex);
    glBindTexture(GL_TEXTURE_2D, stex);
    LoadTGATextureSimple("../resources/groundsm_wht02.tga", &stex);
    glUniform1i(glGetUniformLocation(program, "snowTex"), 1);

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &tex2);
    glBindTexture(GL_TEXTURE_2D, tex2);
   
    LoadTGATextureSimple("../resources/ter_sand1.tga", &tex2);
    glUniform1i(glGetUniformLocation(program, "gtex"), 2);


    glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);


    //terrain

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
    layerLoc = glGetUniformLocation(program, "layer");
    int num = rand() % 8 ;
    
    glUniform1i(layerLoc,num);
    
    
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

  

  
   

    for (int x = 0; x <= 3; x++) {
        for (int z = 0; z <= 3; z++) {
        // Snap offset to player's tile-based grid
        float tileX = floor(cameraPosition.x / ttex.width) * ttex.width;
        float tileZ = floor(cameraPosition.z / ttex.height) * ttex.height;

        
        float offsetX = tileX + x * (ttex.width -1 );
        float offsetZ = tileZ + z *( ttex.height -1 );
        mat4 model = T(offsetX - x, 0, offsetZ -z);
        glUniformMatrix4fv(transMxLoc, 1, GL_TRUE, model.m);
     
        DrawModel(tm, program, "inPosition", "inNormal", "inTexCoord");
        }
    }
      


   

     
    
   

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
    glutCreateWindow("Flying Simulator");
    glutDisplayFunc(display);
    init();
    glutMainLoop();
    return 0;
}
