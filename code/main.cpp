#define MAIN
#include "helperFun.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int vertexBufferObjID;
GLint mdlMxloc, transMxLoc;

void init(void)
{

    // GL inits
    glClearColor(0.4, 0.6, 0.9, 0);
    glutRepeatingTimer(16.67);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    // glutPassiveMotionFunc(mouseMoved);
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

    // locations needed

    mdlMxloc = glGetUniformLocation(program, "mdlMatrix");
    transMxLoc = glGetUniformLocation(program, "trans");
}

void display(void)
{
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

    printError("pre display");

    vec3 cameraUp = Rz((roll * M_PI) / 180) * vec3(0, 1, 0);

    vec3 right = normalize(cross(cameraUp, normalCameraDirection));
    mat3 pitchRotation = rotationMatrix(right, pitch * M_PI / 180.0);
    vec3 pitchedForward = pitchRotation * normalCameraDirection;
    vec3 cameraRight = normalize(cross(pitchedForward, right));
    // cameraUp = normalize(cross(cameraRight, pitchedForward));
    vec3 cameraTarget = cameraPosition + pitchedForward;

    mat4 worldToView = lookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
                              cameraTarget.x, cameraTarget.y, cameraTarget.z,
                              cameraUp.x, cameraUp.y, cameraUp.z);

    mat4 modelToWorld = IdentityMatrix();
    mat4 total = worldToView * modelToWorld;

    glUniformMatrix4fv(mdlMxloc, 1, GL_TRUE, total.m);
    glUniformMatrix4fv(transMxLoc, 1, GL_TRUE, T(0, 0, 0).m);

    DrawModel(tm, program, "inPosition", "inNormal", "inTexCoord");

    printError("display 2");

    keyboardPress();

    /*if (((speed + (throttle - 0.5) / 10) < 1) && ((speed + (throttle - 0.5) / 10) > 0))
        speed += ((throttle - 0.5) / 10);*/

    float dV = throttle * throttleFactor - sin((pitch * M_PI / 180.0) - M_PI) / 2;
    if ((abs(speed + dV) - speed * airDragFactor) < 0)
        speed = 0;
    else if (dV >= 0)
        dV -= speed * airDragFactor;
    else if (dV < 0)
        dV += speed * airDragFactor;
    speed += dV * 0.001;

    // cameraPosition += speed * normalize(vec3(cameraTarget.x - cameraPosition.x, cameraTarget.y - cameraPosition.y, cameraTarget.z - cameraPosition.z));
    cameraPosition += speed * normalize(cameraTarget - cameraPosition) + speed * (roll / 45) * normalize(cross(cameraTarget - cameraPosition, vec3(0.0f, 0.1f, 0.0f)));

    // glutWarpPointer(300, 300);
    glutSwapBuffers();
    // std::cout << roll << " " << speed << " " << throttle << dV << std::endl;
    //   std::cout << cameraND.x << " " << cameraND.y << " " << cameraND.z << std::endl;
    // std::cout << (sin((pitch * M_PI / 180.0) - M_PI) / 2) << std::endl;

    // console:
    std::cout << "Speed: " << speed << std::endl;
    std::cout << "Throttle: " << throttle * 100 << "%" << std::endl;
    std::cout << "Altitude: " << "TODO!" << std::endl;
    std::cout << "Roll: " << roll << " degrees" << std::endl;
    std::cout << "Pitch: " << -1 * pitch << " degrees" << std::endl;
    std::cout << "\033[2J"; // clear screen
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion(3, 2);
    // glutInitWindowSize(1920, 1080);
    glutInitWindowSize(800, 600);
    // glutToggleFullScreen();
    glutCreateWindow("Flight simulator");
    glutDisplayFunc(display);
    init();
    glutMainLoop();
    return 0;
}
