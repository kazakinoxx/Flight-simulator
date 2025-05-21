#define MAIN
#include "helperFun.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

struct InstanceData
{
    vec3 position;
    float layer;
};

void updatePhysics()
{

    vec3 cameraUp = Rz((roll * M_PI) / 180) * vec3(0, 1, 0);

    vec3 right = normalize(cross(cameraUp, normalCameraDirection));
    mat3 pitchRotation = rotationMatrix(right, pitch * M_PI / 180.0);
    vec3 pitchedForward = pitchRotation * normalCameraDirection;
    vec3 cameraTarget = cameraPosition + pitchedForward;

    float dV = throttle * throttleFactor - sin((pitch * M_PI / 180.0) - M_PI) / 2;
    if ((abs(speed + dV) - speed * airDragFactor) < 0)
        speed = 0;
    else if (dV >= 0)
        dV -= speed * airDragFactor;
    else if (dV < 0)
        dV += speed * airDragFactor;
    speed += dV * 0.001;

    cameraPosition += speed * normalize(cameraTarget - cameraPosition) +
                      speed * (roll / 45) * normalize(cross(cameraTarget - cameraPosition, vec3(0.0f, 0.1f, 0.0f)));
}

void printDebugInfo()
{
    std::cout << "Speed: " << speed << std::endl;
    std::cout << "Throttle: " << throttle * 100 << "%" << std::endl;
    std::cout << "Altitude: " << cameraPosition.y << std::endl;
    std::cout << "Roll: " << roll << " degrees" << std::endl;
    std::cout << "Pitch: " << -1 * pitch << " degrees" << std::endl;
    std::cout << "\033[2J"; // clear screen
}
std::vector<InstanceData> instances;

unsigned int vertexBufferObjID;

GLint mdlMxloc, transMxLoc;
Model *skybox;
Model *cockpitQuad;

unsigned int SkyTex;
GLuint cockpitTex;
GLuint overlayprogram;

GLuint cockpitVBO, cockpitVAO;
GLuint vao;
// 2 triangles forming a rectangle
float cockpitVerts[] = {
    // x,    y,   z,   u,  v
    -1.0f, -1.0f, 0, 0.0f, 0.0f, // bottom left
    1.0f, -1.0f, 0, 1.0f, 0.0f,  // bottom right
    1.0f, -0.3f, 0, 1.0f, 1.0f,  // top right

    -1.0f, -1.0f, 0, 0.0f, 0.0f, // bottom left
    1.0f, -0.3f, 0, 1.0f, 1.0f,  // top right
    -1.0f, -0.3f, 0, 0.0f, 1.0f  // top left
};

GLuint instanceVBO;

TextureData textures[8];
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

    // Set the sampler uniform
    glUniform1i(glGetUniformLocation(program, "snowTex"), 1);

    GLint width = 512;
    GLint height = 512;
    int numLayer = 7;

    // Allocate immutable storage
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, width, height, numLayer); // Use GL_RGB8 if bpp = 24

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Upload each image to a layer
    for (int i = 0; i < numLayer; i++)
    {
        char filename[64];
        sprintf(filename, "../resources/groundsm_wht0%i.tga", i + 1);

        LoadTGATexture(filename, &textures[i]);

        if (textures[i].bpp != 24)
        {
            printf("ERROR: Unexpected bpp in texture %d: %d\n", i, textures[i].bpp);
            continue;
        }

        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0,
            0, 0, i,          // x, y, z offsets
            width, height, 1, // width, height, depth
            GL_RGB,           // format must match bpp
            GL_UNSIGNED_BYTE,
            textures[i].imageData);

        free(textures[i].imageData);
    }

    // // Fill instances
    // for (int x = 0; x < width; ++x) {
    //     for (int z = 0; z < height; ++z) {
    //         InstanceData data;
    //         data.position = vec3(x * 5, 0, z * 5);
    //         data.layer = rand() % 7;
    //         instances.push_back(data);
    //     }
    // }

    // // Generate and bind buffer
    // GLuint instanceVBO;
    // glGenBuffers(1, &instanceVBO);
    // glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_STATIC_DRAW);

    // glEnableVertexAttribArray(2);
    // glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, position));
    // glVertexAttribDivisor(2, 1);

    // glEnableVertexAttribArray(3);
    // glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, layer));
    // glVertexAttribDivisor(3, 1);

    // glGenVertexArrays(1, &vao);
    // glBindVertexArray(vao);

    // // Bind instanceVBO (already declared globally)
    // glGenBuffers(1, &instanceVBO);
    // glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_STATIC_DRAW);

    // // Query locations after program linked
    // GLuint posAttrib = glGetAttribLocation(program, "instancePosition");
    // GLuint layerAttrib = glGetAttribLocation(program, "instanceLayer");

    // // Setup vertex attrib pointers for instance data
    // glEnableVertexAttribArray(posAttrib);
    // glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, position));
    // glVertexAttribDivisor(posAttrib, 1);

    // glEnableVertexAttribArray(layerAttrib);
    // glVertexAttribPointer(layerAttrib, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, layer));
    // glVertexAttribDivisor(layerAttrib, 1);

    // glBindVertexArray(0);

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

    // glUseProgram(overlayprogram);
    glGenVertexArrays(1, &cockpitVAO);
    glGenBuffers(1, &cockpitVBO);

    glBindVertexArray(cockpitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cockpitVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cockpitVerts), cockpitVerts, GL_STATIC_DRAW);

    // Position (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // TexCoord (2 floats)
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
    // Clear the screen (set clear color BEFORE clearing)
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update time
    t = (GLfloat)glutGet(GLUT_ELAPSED_TIME) / 100;

    int layer = (int)(t / 50) % 6;
    glUniform1i(glGetUniformLocation(program, "layer"), layer);

    // Update instance data if needed
    // glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData),
    //             instances.data(), GL_DYNAMIC_DRAW);

    // ------ MAIN RENDER PASS ------
    glUseProgram(program);

    // Convert input angles to radians
    float pitchRad = deltaPitch * M_PI / 180.0f;
    float rollRad = deltaRoll * M_PI / 180.0f;
    deltaPitch = 0;
    deltaRoll = 0;

    // Get current local axes from orientation matrix
    vec3 forward = normalize(vec3(orientation * vec4(0, 0, -1, 0)));
    vec3 right = normalize(vec3(orientation * vec4(1, 0, 0, 0)));

    // Apply pitch: rotate around current local right
    mat4 pitchMatrix = rotationMatrix(right, pitchRad);
    orientation = pitchMatrix * orientation;

    // Recompute forward after pitch
    forward = normalize(vec3(orientation * vec4(0, 0, -1, 0)));

    // Apply roll: rotate around updated forward axis
    mat4 rollMatrix = rotationMatrix(forward, rollRad);
    orientation = rollMatrix * orientation;

    // Get final camera basis
    forward = normalize(vec3(orientation * vec4(0, 0, -1, 0)));
    vec3 up = normalize(vec3(orientation * vec4(0, 1, 0, 0)));

    // Get camera target and view WTW matrix
    vec3 cameraTarget = cameraPosition + forward;

    mat4 worldToView = lookAt(cameraPosition, cameraTarget, up);

    mat4 modelToWorld = IdentityMatrix();
    mat4 total = worldToView * modelToWorld;
    glUniformMatrix4fv(mdlMxloc, 1, GL_TRUE, total.m);

    // Render skybox
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    mat4 skyModel = T(cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniformMatrix4fv(glGetUniformLocation(program, "trans"), 1, GL_TRUE, skyModel.m);
    glUniform1i(glGetUniformLocation(program, "texUnit"), 3);
    DrawModel(skybox, program, "inPosition", "inNormal", "inTexCoord");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glUniform1i(glGetUniformLocation(program, "texUnit"), 0);

    // Render terrain tiles
    for (int x = 0; x <= 3; x++)
    {
        for (int z = 0; z <= 3; z++)
        {
            float tileX = floor(cameraPosition.x / ttex.width) * ttex.width;
            float tileZ = floor(cameraPosition.z / ttex.height) * ttex.height;
            float offsetX = tileX + x * (ttex.width - 1);
            float offsetZ = tileZ + z * (ttex.height - 1);
            mat4 model = T(offsetX - x, 0, offsetZ - z);
            glUniformMatrix4fv(transMxLoc, 1, GL_TRUE, model.m);
            DrawModel(tm, program, "inPosition", "inNormal", "inTexCoord");
        }
    }

    // Render instances (uncomment and fix if you want to use this)
    // glBindVertexArray(vao);
    // glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, (GLsizei)instances.size());
    // glBindVertexArray(0);

    // ------ OVERLAY RENDER PASS ------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(overlayprogram);
    glDisable(GL_DEPTH_TEST);

    mat4 ort = ortho(-1, 1, -1, 1, -1, 1);
    glUniformMatrix4fv(glGetUniformLocation(overlayprogram, "proj"), 1, GL_TRUE, ort.m);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, cockpitTex);
    glUniform1i(glGetUniformLocation(overlayprogram, "tex"), 4);

    glBindVertexArray(cockpitVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Reset state
    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);

    // Update physics and input
    keyboardPress();
    // updatePhysics();

    float dV = throttle * throttleFactor - sin((pitch * M_PI / 180.0) - M_PI) / 2;
    if ((abs(speed + dV) - speed * airDragFactor) < 0)
        speed = 0;
    else if (dV >= 0)
        dV -= speed * airDragFactor;
    else if (dV < 0)
        dV += speed * airDragFactor;
    speed = speed + dV * 0.001 > 1 ? 1 : speed + dV * 0.001;
    sidewaysSpeed = (abs(roll) > 15 && abs(roll) < 165) ? ((upsideDownPitch || upsideDownRoll) ? (roll < 0.0 ? 180.0 + roll : 180.0 - roll) : roll) : 0;

    cameraPosition += speed * normalize(cameraTarget - cameraPosition) + (sidewaysSpeed / 360.0) * normalize(cross(cameraTarget - cameraPosition, vec3(0.0f, 0.1f, 0.0f)));

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
    glutInitWindowSize(800, 600);
    glutCreateWindow("Flight simulator");
    glutDisplayFunc(display);
    init();
    glutMainLoop();
    return 0;
}
