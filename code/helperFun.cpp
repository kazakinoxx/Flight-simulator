#include "helperFun.h"

GLfloat t;

GLuint program;
mat4 projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 5000.0);
vec3 *vertexArray;
Model *tm;
GLuint tex1, tex2, stex;
TextureData ttex;
// double movementSpeed = 0.5;
vec3 cameraDirection;
vec3 normalCameraDirection(0.683106f, 0.321681f, 0.655658f);
vec3 cameraPosition(50.0f, 20.f, -100.0f);
// vec3 cameraRotation(0.0f, 1.0f, 0.0f);
mat4 orientation = mat4(1.0f);

vec2 direction(0.0f, 0.0f);
// physics
float throttle = 0.05; // 0 to 1
float throttleFactor = 0.75;
float speed = 0.05; // 0 to 1
float sidewaysSpeed = 0;
float airDragFactor = 0.35;
float verticalDragFactor = 0.25;
int lastRoll = 0;
int lastPitch = 0;
int roll = 0;
int pitch = 0;
int deltaRoll = 0;
int deltaPitch = 0;
bool upsideDownRoll = false;
bool upsideDownPitch = false;

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
			vertexArray[(x + z * tex->width)].y = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / 2.0;
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

void keyboardPress()
{
	/*
	if (glutKeyIsDown('d'))
		cameraPosition += movementSpeed * normalize(cross(normalCameraDirection, vec3(0.0f, 0.1f, 0.0f)));
	else if (glutKeyIsDown('a'))
		cameraPosition -= movementSpeed * normalize(cross(normalCameraDirection, vec3(0.0f, 0.1f, 0.0f)));
	else if (glutKeyIsDown('w'))
		cameraPosition += movementSpeed * normalCameraDirection;
	else if (glutKeyIsDown('s'))
		cameraPosition -= movementSpeed * normalCameraDirection;
	*/

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
		if (pitch == 89)
		{
			upsideDownPitch = true;
			pitch = 90;
		}
		else if (pitch == 180)
			pitch = -179;
		else if (pitch == -90)
		{
			upsideDownPitch = false;
			pitch = -89;
		}
		else
			pitch += 1;
	}

	else if (glutKeyIsDown('w'))
	{
		if (pitch == -89)
		{
			upsideDownPitch = true;
			pitch = -90;
		}
		else if (pitch == -179)
			pitch = 180;
		else if (pitch == 90)
		{
			upsideDownPitch = false;
			pitch = 89;
		}
		else
			pitch -= 1;
	}
	deltaPitch += pitch - lastPitch;
	deltaRoll += roll - lastRoll;
	lastPitch = pitch;
	lastRoll = roll;

	if (glutKeyIsDown('r'))
		throttle += 1;
	else if (glutKeyIsDown('f') && throttle > 0.0)
		throttle -= 0.05;
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

/*void mouseMoved(int x, int y)
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
}*/