#include "helperFun.h"

GLfloat t;

GLuint program;
mat4 projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 5000.0);
vec3 *vertexArray;
Model *tm;
GLuint tex1, tex2, stex;
TextureData ttex;
vec3 cameraDirection(1.0f, 0, 1.0f);
vec3 normalCameraDirection(0.683106f, 0.321681f, 0.655658f);
vec3 cameraPosition(50.0f, 20.f, -100.0f);
mat4 orientation = mat4(1.0f);
vec3 forward;
vec3 right;
vec3 up;
mat4 pitchMatrix;
mat4 rollMatrix;
mat4 turnMatrix;
vec3 cameraTarget;
vec2 direction(0.0f, 0.0f);

float throttle = 0.05; // 0 to 1
float throttleFactor = 1.0;
float dV = 0.0;
float speed = 0.05; // 0 to 1
float sidewaysSpeed = 0;
float airDragFactor = 0.25;
float verticalDragFactor = 0.55;
int lastRoll = 0;
int lastPitch = 0;
int roll = 0;
int pitch = 0;
float rollRad = 0.0;
float pitchRad = 0.0;
int deltaRoll = 0;
int deltaPitch = 0;
float absRollRad = 0.0;
bool upsideDownRoll = false;
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

GLuint instanceVBO;

TextureData textures[8];


struct InstanceData
{
    vec3 position;
    float layer;
};

void printDebugInfo()
{
    std::cout << "Speed: " << speed << std::endl;
    std::cout << "Throttle: " << throttle * 100 << "%" << std::endl;
    std::cout << "Altitude: " << cameraPosition.y << std::endl;
    std::cout << "Roll: " << roll << " degrees" << std::endl;
    std::cout << "Pitch: " << pitch << " degrees" << std::endl;
    std::cout << "\033[2J"; // clear screen
}

mat4 rotationMatrix(vec3 axis, float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	float t = 1.0 - c;

	axis = normalize(axis);
	float x = axis.x, y = axis.y, z = axis.z;

	return mat4(
		t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0,
		t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0,
		t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0,
		0, 0, 0, 1);
}

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
			// Vertex array.
			vertexArray[(x + z * tex->width)].x = x / 1.0;
			vertexArray[(x + z * tex->width)].y = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / 2.0;
			vertexArray[(x + z * tex->width)].z = z / 1.0;
			// Normal vectors.
			normalArray[(x + z * tex->width)].x = 0.0;
			normalArray[(x + z * tex->width)].y = 1.0;
			normalArray[(x + z * tex->width)].z = 0.0;

			//  Texture coordinates.
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

void keyboardPress()
{
	if (glutKeyIsDown('d'))
	{
		if (roll == 89)
		{
			upsideDownRoll = true;
			roll = 90;
		}
		else if (roll == 180)
			roll = -179;
		else if (roll == -90)
		{
			upsideDownRoll = false;
			roll = -89;
		}
		else
			roll += 1;
	}

	else if (glutKeyIsDown('a'))
	{
		if (roll == -89)
		{
			upsideDownRoll = true;
			roll = -90;
		}
		else if (roll == -179)
			roll = 180;
		else if (roll == 90)
		{
			upsideDownRoll = false;
			roll = 89;
		}
		else
			roll -= 1;
	}

	if (glutKeyIsDown('s'))
	{
		if (pitch != 89)
			pitch += 1;
	}

	else if (glutKeyIsDown('w'))
	{
		if (pitch != -89)
			pitch -= 1;
	}
	deltaPitch += pitch - lastPitch;
	deltaRoll += roll - lastRoll;
	lastPitch = pitch;
	lastRoll = roll;

	if (glutKeyIsDown('r') && throttle + 0.05 <= 1.01)
		throttle += 0.05;
	else if (glutKeyIsDown('f') && throttle - 0.05 >= -0.01)
		throttle -= 0.05;
}

void updatePhysics()
{
	pitchRad = deltaPitch * M_PI / 180.0f;
	rollRad = deltaRoll * M_PI / 180.0f;
	absRollRad = ((upsideDownRoll) ? (roll < 0.0 ? 180.0 + roll : 180.0 - roll) : roll) * M_PI / 180.0f;

	deltaPitch = 0;
	deltaRoll = 0;

	forward = normalize(vec3(orientation * vec4(0, 0, -1, 0)));
	right = normalize(vec3(orientation * vec4(1, 0, 0, 0)));

	pitchMatrix = rotationMatrix(right, pitchRad);
	orientation = pitchMatrix * orientation;

	forward = normalize(vec3(orientation * vec4(0, 0, -1, 0)));

	rollMatrix = rotationMatrix(forward, rollRad);
	orientation = rollMatrix * orientation;

	forward = normalize(vec3(orientation * vec4(0, 0, -1, 0)));

	// up = normalize(vec3(orientation * vec4(0, -1, 0, 0)));
	turnMatrix = rotationMatrix(vec3(0, -1, 0), speed * absRollRad / 90.0);
	orientation = turnMatrix * orientation;

	forward = normalize(vec3(orientation * vec4(0, 0, -1, 0)));
	up = normalize(vec3(orientation * vec4(0, 1, 0, 0)));

	dV = throttle * throttleFactor + verticalDragFactor * (sin((pitch * M_PI / 180.0) - M_PI) / 2);
	if ((abs(speed + dV) - speed * airDragFactor) < 0)
		speed = 0;
	else if (dV >= 0)
		dV -= speed * airDragFactor;
	else if (dV < 0)
		dV += speed * airDragFactor;
	speed = speed + dV * 0.005 > 0.995 ? 0.995 : speed + dV * 0.005;

	// sidewaysSpeed = (abs(roll) > 15 && abs(roll) < 165) ? ((upsideDownPitch || upsideDownRoll) ? (roll < 0.0 ? 180.0 + roll : 180.0 - roll) : roll) : 0;
	sidewaysSpeed = sin(((upsideDownRoll) ? (roll < 0.0 ? 180.0 + roll : 180.0 - roll) : roll) * M_PI / 180.0);
	// sidewaysSpeed = 0.0;

	cameraPosition += speed * normalize(cameraTarget - cameraPosition) + speed * sidewaysSpeed * normalize(cross(cameraTarget - cameraPosition, vec3(0.0f, 0.1f, 0.0f)));

	cameraTarget = cameraPosition + forward;
}

 void multiGen(){
    // terrain
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
 }


void multiBind(int numLayer, int width, int height){

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
            0, 0, i,          
            width, height, 1, 
            GL_RGB,
            GL_UNSIGNED_BYTE,
            textures[i].imageData);

        free(textures[i].imageData);
    }

}