// THE WALL
// Demo of simple Portals
// 2. The corridor with visible portals.
// Similar to the wall, just one cell but now seeminly infinite.

#include "MicroGlut.h"
//uses framework Cocoa
//uses framework OpenGL
#define MAIN
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"

mat4 projectionMatrix;

Model *cube;
GLuint grasstex, rocktex, woodtex;

// Reference to shader programs
GLuint texShader, portalShader;

typedef struct CubeRec
{
	vec3 pos;
	vec3 scale;
	vec3 minp;
	vec3 maxp;
	GLint tex;
	Model *model;
} CubeRec;

int cubeCount = 0;
struct CubeRec *cubes; 


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// Given an AABB, push along shortest axis!
vec3 ShortestAxis(vec3 minp, vec3 maxp, vec3 b, float *outd, float camsize)
{
	float mindist = 1000000;
	vec3 v = vec3(0,0,0);
	float d;
	int hit = 1;
	
	minp.x -= camsize;
	minp.y -= camsize;
	minp.z -= camsize;
	maxp.x += camsize;
	maxp.y += camsize;
	maxp.z += camsize;
	
	d = b.x - minp.x;
	if (d < 0)
		hit = 0;
	else
	if (d < mindist)
	{
		mindist = d;
		v = vec3(-d, 0, 0);
	}

	d = maxp.x - b.x;
	if (d < 0)
		hit = 0;
	else
	if (d < mindist)
	{
		mindist = d;
		v = vec3(d, 0, 0);
	}

	d = b.y - minp.y;
	if (d < 0)
		hit = 0;
	else
	if (d < mindist)
	{
		mindist = d;
		v = vec3(0, -d, 0);
	}

	d = maxp.y - b.y;
	if (d < 0)
		hit = 0;
	else
	if (d < mindist)
	{
		mindist = d;
		v = vec3(0, d, 0);
	}

	d = b.z - minp.z;
	if (d < 0)
		hit = 0;
	else
	if (d < mindist)
	{
		mindist = d;
		v = vec3(0, 0, -d);
	}

	d = maxp.z - b.z;
	if (d < 0)
		hit = 0;
	else
	if (d < mindist)
	{
		mindist = d;
		v = vec3(0, 0, d);
	}

	if (hit)
		*outd = mindist;
	else
		*outd = 1000000;
	
	return v;
}

void addBox(float x1, float x2, float y1, float y2, float z1, float z2, GLint tex, Model *model)
{
	if (cubeCount == 0)
		cubes = (CubeRec *)malloc(sizeof(CubeRec));
	else
		cubes = (CubeRec *)realloc(cubes, (cubeCount+1)*sizeof(CubeRec));
	
	cubes[cubeCount].pos = vec3((x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2);
	cubes[cubeCount].scale = vec3(fabs(x1 - x2) / 2, fabs(y1 - y2) / 2, fabs(z1 - z2) / 2);
	cubes[cubeCount].tex = tex;
	cubes[cubeCount].model = model;
	cubes[cubeCount].minp = vec3(min(x1, x2), min(y1, y2), min(z1, z2));
	cubes[cubeCount].maxp = vec3(max(x1, x2), max(y1, y2), max(z1, z2));

	cubeCount += 1;
}

typedef struct Portal
{
	vec3 a, b, c, d; // 4 corners
	mat4 trans;
	mat4 rot;
	vec3 center;
	int dest; // Future use: Which cell is this?
	Model *model;
} Portal;

// With only one cell and one portal, here is the only one:
Portal *p;


void init(void)
{
	// GL inits
	glClearColor(0,0,0,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300.0);

	// Load and compile shader
	portalShader = loadShaders("portal.vert", "portal.frag");
	texShader = loadShadersG("textured.vert", "textured.frag", "textured.gs");
	printError("init shader");
	
	// Upload geometry to the GPU:
	cube = LoadModel("cube.obj");

// Important! The shader we upload to must be active!

	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(texShader, "tex"), 0); // Texture unit 0

	glUseProgram(portalShader);
	glUniformMatrix4fv(glGetUniformLocation(portalShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(portalShader, "tex"), 0); // Texture unit 0

	LoadTGATextureSimple("grass.tga", &grasstex);
	LoadTGATextureSimple("rock.tga", &rocktex);
	LoadTGATextureSimple("wood.tga", &woodtex);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, rocktex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);


	// The corridor
	addBox(-5,-3, 0,6, -0.2,0.2, rocktex, cube);
	addBox(3,5, 0,6, -0.2,0.2, rocktex, cube);
	addBox(-3,3, 3,6, -0.2,0.2, rocktex, cube); // Above portal

	addBox(-5,-4.8, 0,6, 0,20, rocktex, cube); // Side wall
	addBox(5,4.8, 0,6, 0,20, rocktex, cube); // Side wall

	addBox(-5,5, 6,6.2, 0,20, rocktex, cube); // Ceiling
	addBox(-5,5, -0.1,0, 0,20, grasstex, cube); // Floor

	// Init the portal
	p = (Portal *) calloc(sizeof(Portal), 1);
	p->a = vec3(-3, 0, 0);
	p->b = vec3(3, 0, 0);
	p->c = vec3(-3, 3, 0);
	p->d = vec3(3, 3, 0);
	p->trans = T(0,0,20);
	p->rot = IdentityMatrix();
	p->center = vec3(0,0,0);

	// Create portal for drawing
	vec3 verticesp[] = {p->a, p->b, p->d, p->c};
	GLfloat texcoordp[] = {	M_PI, M_PI,
						0.0f, M_PI,
						0.0f, 0.0f,
						M_PI, 0.0f};
	GLuint indicesp[] = {	0,2,3, 0,1,2};
	
	p->model = LoadDataToModel((vec3 *)verticesp, NULL, (vec2 *)texcoordp, NULL,
			indicesp, 4, 6);

	printError("init arrays");
}

GLfloat a = 0.0;
vec3 campos = vec3(0, 0.5, 14);
vec3 forward = vec3(0, 0, -4);
vec3 up = vec3(0, 1, 0);

#define kCamSize 1.0

void Move()
{
	vec3 oldpos = campos; // To check whether we pass through a portal!
	
	if (glutKeyIsDown('a'))
		forward = mat3(Ry(0.03)) * forward;
	if (glutKeyIsDown('d'))
		forward = mat3(Ry(-0.03)) * forward;
	if (glutKeyIsDown('w'))
		campos = campos + forward * 0.05;
	if (glutKeyIsDown('s'))
		campos = campos - forward * 0.05;
	if (glutKeyIsDown('q'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		campos = campos - side * 0.05;
	}
	if (glutKeyIsDown('e'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		campos = campos + side * 0.05;
	}

	// Move up/down
	if (glutKeyIsDown('z'))
		campos = campos + vec3(0,1,0) * 0.01;
	if (glutKeyIsDown('c'))
		campos = campos - vec3(0,1,0) * 0.01;

	// NOTE: Looking up and down is done by making a side vector and rotation around arbitrary axis!
	if (glutKeyIsDown('+'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		mat4 m = ArbRotate(side, 0.05);
		forward = mat3(m) * forward;
	}
	if (glutKeyIsDown('-'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		mat4 m = ArbRotate(side, -0.05);
		forward = mat3(m) * forward;
	}
	
	// Collision detection with cubes!
	// The aabb was pre-calculated by addCube().
	vec3 close = vec3(0,0,0);
	float mindist = 1000000;
	float d = 0.0;
	for (int i = 0; i < cubeCount; i++) // for every cube
	{
		vec3 pp = ShortestAxis(cubes[i].minp, cubes[i].maxp, campos, &d, kCamSize);
		if (d < 1000000)
		{
			mindist = d;
			if (pp.x != close.x || pp.y != close.y || pp.z != close.z)
				close = pp + close; // Better but not perfect.
		}
	}

	if (mindist < kCamSize)
	{
		// Create vector with length kCamSize - mindist
		vec3 push = close * (kCamSize - mindist)/mindist;
		push = close;
		campos = campos + push;
	}

	if (campos.y < kCamSize + 0.5)
		campos.y = kCamSize + 0.5;
	
	// Check portal!
	
	// One layer of {} is unnecessary, it is there for the future multiple portals,
	// but also to allow me to use the variable d again.
	{
		vec3 a = p->a;
		vec3 b = p->b;
		vec3 c = p->c;
		vec3 d = p->d;
		// Test assumed portals are axis aligned!
		if ((a.x == b.x) && (b.x == c.x) && (c.x == d.x)) // Portal along z
		{
			if ((a.x < campos.x) != (a.x < oldpos.x))
			// Must also check borders
			if (((a.z - campos.z)*(campos.z - b.z)) >= 0)
			if (((a.y - campos.y)*(campos.y - c.y)) >= 0)
			{
				// Passed through!
				// campos is not guaranteed to be in the center!
				// We must rotate around the center of the portal.
				campos = campos - p->center; // Rotate around portal
				campos = p->rot * campos;
				campos = campos + p->center;
														// Then translate to portal destination
				campos = p->trans * campos;
				forward = p->rot * forward; // Also rotate rotation!

				printf("SWITCH X\n");
			}
		}
		else
		if ((a.z == b.z) && (b.z == c.z) && (c.z == d.z)) // Portal along x
		{
			if ((a.z < campos.z) != (a.z < oldpos.z))
			// Must also check borders
			if (((a.x - campos.x)*(campos.x - b.x)) >= 0)
			if (((a.y - campos.y)*(campos.y - c.y)) >= 0)
			{
				campos = campos - p->center; // Rotate around portal
				campos = p->rot * campos;
				campos = campos + p->center;
														// Then translate to portal destination
				campos = p->trans * campos;
				forward = p->rot * forward; // Also rotate rotation!
				
				printf("SWITCH Z\n");
			}
		}
// Y skipped, we don't expect portals in floor or ceilings - yet
	}

	
}

void DrawScene(mat4 worldToView, int count) // Draw cells recursively! Needed for the semitransparent portal!
{
	mat4 m;
	
	if (count < 5)
		DrawScene(Mult(worldToView, Mult(InvertMat4(p->trans), p->rot)), count+1);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	for (int i = 0; i < cubeCount; i++)
	{
		// Cube
		glUseProgram(texShader);
		glBindTexture(GL_TEXTURE_2D, cubes[i].tex);
		m = worldToView * T(cubes[i].pos.x,cubes[i].pos.y,cubes[i].pos.z);
		m = m * S(cubes[i].scale.x,cubes[i].scale.y,cubes[i].scale.z);
		glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
		glUniform3fv(glGetUniformLocation(texShader, "scaling"), 1, (const GLfloat *)&cubes[i].scale);
		DrawModel(cubes[i].model, texShader, "inPosition", "inNormal", "inTexCoord");
	}

	glEnable(GL_BLEND);
	
	glUseProgram(portalShader);
	glUniformMatrix4fv(glGetUniformLocation(portalShader, "modelviewMatrix"), 1, GL_TRUE, worldToView.m);
	DrawModel(p->model, portalShader, "inPosition", NULL, "inTexCoord");
}

void display(void)
{
	int i, j;
	
	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	mat4 worldToView, m; // m1, m2, m, tr, scale;
	
	Move();

	worldToView = lookAtv(campos, campos + forward, up);
	
	a += 0.1;

	// Current
	DrawScene(worldToView, 0); // Draw current cell recursively
	
	printError("display");
	
	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(800,800);
	glutCreateWindow ("2. The corridor");
	glutRepeatingTimer(20);
	glutDisplayFunc(display); 
	glutKeyboardFunc(keys);
	init ();
	glutMainLoop();
}
