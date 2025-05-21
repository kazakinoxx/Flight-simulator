// Demo of simple Portals
// 3. Multiple rooms
// Multiple, convex cells. Can be a simple case if we can't see overlaps.

// Two rooms. No portals backwards. No checking for duplicates.

#include "MicroGlut.h"
//uses framework Cocoa
//uses framework OpenGL
#define MAIN
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"

mat4 projectionMatrix;

Model *cube, *halfcube;
GLuint grasstex, rocktex, woodtex, bluegrasstex;
int currentCell = 0;

// Reference to shader programs
GLuint texShader, portalShader;

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

typedef struct Portal
{
	vec3 a, b, c, d; // 4 corners
	mat4 trans;
	mat4 rot;
	vec3 center;
	int dest; // Which cell does it lead to?
	Model *model;
} Portal;

typedef struct CubeRec
{
	vec3 pos;
	vec3 scale;
	vec3 minp;
	vec3 maxp;
	GLint tex;
	Model *model;
} CubeRec;

typedef struct CellRec
{
	int cubeCount; //  = 0;
	struct CubeRec *cubes;
	int portalCount; //  = 0;
	Portal *portals;
} CellRec;

// Array of cells
CellRec *cells = NULL;
int cellsCount = 0;

void addBox(int cell, float x1, float x2, float y1, float y2, float z1, float z2, GLint tex, Model *model)
{
	if (cells[cell].cubeCount == 0)
		cells[cell].cubes = (CubeRec *)malloc(sizeof(CubeRec));
	else
		cells[cell].cubes = (CubeRec *)realloc(cells[cell].cubes, (cells[cell].cubeCount+1)*sizeof(CubeRec));
	
	cells[cell].cubes[cells[cell].cubeCount].pos = vec3((x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2);
	cells[cell].cubes[cells[cell].cubeCount].scale = vec3(fabs(x1 - x2) / 2, fabs(y1 - y2) / 2, fabs(z1 - z2) / 2);
	cells[cell].cubes[cells[cell].cubeCount].tex = tex;
	cells[cell].cubes[cells[cell].cubeCount].model = model;
	cells[cell].cubes[cells[cell].cubeCount].minp = vec3(min(x1, x2), min(y1, y2), min(z1, z2));
	cells[cell].cubes[cells[cell].cubeCount].maxp = vec3(max(x1, x2), max(y1, y2), max(z1, z2));

	cells[cell].cubeCount += 1;
}

void addPortal(int cell, int toCell, vec3 a, vec3 b, vec3 c, vec3 d, mat4 trans, mat4 rot)
{
	int portalCount = cells[cell].portalCount;
	if (cells[cell].portals == NULL)
		cells[cell].portals = (Portal *) malloc(sizeof(Portal));
	else
		cells[cell].portals = (Portal *) realloc(cells[cell].portals, (portalCount+1)*sizeof(Portal));
		
	cells[cell].portals[portalCount].a = a;
	cells[cell].portals[portalCount].b = b;
	cells[cell].portals[portalCount].c = c;
	cells[cell].portals[portalCount].d = d;
	cells[cell].portals[portalCount].trans = trans;
	cells[cell].portals[portalCount].rot = rot;
	cells[cell].portals[portalCount].center = vec3((a.x+b.x+c.x+d.x)/4, 0, (a.z + b.z + c.z + d.z)/4);

	cells[cell].portals[portalCount].dest = toCell;


	// Create portal for drawing
	vec3 verticesp[] = {cells[cell].portals[portalCount].a, cells[cell].portals[portalCount].b, cells[cell].portals[portalCount].d, cells[cell].portals[portalCount].c};
	GLfloat texcoordp[] = {	M_PI, M_PI,
						0.0f, M_PI,
						0.0f, 0.0f,
						M_PI, 0.0f};
	GLuint indicesp[] = {	0,2,3, 0,1,2};
	
	cells[cell].portals[portalCount].model = LoadDataToModel((vec3 *)verticesp, NULL, (vec2 *)texcoordp, NULL,
			indicesp, 4, 6);


	cells[cell].portalCount += 1;
}

int addCell(int n)
{
	for (int i = 0; i < n; i++)
	{
		if (cells == NULL)
			cells = (CellRec *) calloc(sizeof(CellRec), 1);
		else
			cells = (CellRec *) realloc(cells, (cellsCount+1)*(sizeof(CellRec)));

		cells[cellsCount].cubeCount = 0;
		cells[cellsCount].cubes = NULL;
		cells[cellsCount].portalCount = 0;
		cells[cellsCount].portals = NULL;
		cellsCount += 1;
	}
	return cellsCount-1; // Index of last cell added
}

	
void buildStraightCell(int thisCell, int northCell, int southCell, GLuint floorTex)
{	
// ROOM 0: Straight section.

	// Front
	addBox(thisCell, -5,-3, 0,7,  -0.2,0.2, rocktex, cube);
	addBox(thisCell, 3,5, 0,7,  -0.2,0.2, rocktex, cube);
	addBox(thisCell, -3,3, 3,7, -0.2,0.2, rocktex, cube);
	// Back
	addBox(thisCell, -5,-3, 0,7,  19.8,20.2, rocktex, cube);
	addBox(thisCell, 3,5, 0,7,  19.8,20.2, rocktex, cube);
	addBox(thisCell, -3,3, 3,7, 19.8,20.2, rocktex, cube);
	// Sides
	addBox(thisCell, -5.3,-4.7, 0,7,  0,20, rocktex, cube);
	addBox(thisCell, 4.7,5.3, 0,7,  0,20, rocktex, cube);
	// Ceiling and floor
	addBox(thisCell, -5,5, 6.8,7.2, 0,20, woodtex, cube);
	addBox(thisCell, -5,5, -0.2,0.0, 0,20, floorTex, cube);

	// Init the portals
	
	// Front portal
	addPortal(thisCell,	// Add to this cell
		northCell,			// Destination cell
		vec3(-3, 0, 0), // Four corners of portal. Must be a square.
		vec3(3, 0, 0),
		vec3(-3, 3, 0),
		vec3(3, 3, 0),
		T(0,0,10), // Translation part of portal transformation
		IdentityMatrix()); // Rotation part of portal transformation
	
	// Back portal - portal to right
	if (0) // Skip - it causes problems for this case
	addPortal(thisCell,	// Add to this cell
		southCell,			// Destination cell
		vec3(3, 0, 20), // Four corners of portal. Must be a square.
		vec3(-3, 0, 20),
		vec3(3, 3, 20),
		vec3(-3, 3, 20),
		T(5,0,-15), // Translation part of portal transformation
		Ry(-M_PI/2)); // Rotation part of portal transformation
}


void buildCornerCell(int thisCell, int westCell, int southCell, GLuint floorTex)
{
// ROOM 1: Corner section

	// Front wall
	addBox(thisCell, -5,5, 0,7,  -0.2,0.2, rocktex, cube);
	// Right portal
	addBox(thisCell, 4.8,5.2, 0,7,  0,2, rocktex, cube);
	addBox(thisCell, 4.8,5.2, 0,7,  8,10, rocktex, cube);
	addBox(thisCell, 4.8,5.2, 3,7, 2,8, rocktex, cube);
	// Back (portal)
	addBox(thisCell, -5,-3, 0,7,  9.8,10.2, rocktex, cube);
	addBox(thisCell, 3,5, 0,7,  9.8,10.2, rocktex, cube);
	addBox(thisCell, -3,3, 3,7, 9.8,10.2, rocktex, cube);
	// Left side
	addBox(thisCell, -5.3,-4.7, 0,7,  0,10, rocktex, cube);
	// Ceiling
	addBox(thisCell, -5,5, 6.8,7.2, 0,10, rocktex, cube);
	// Floor
	addBox(thisCell, -5,5, 0, -0.2, 0,10, floorTex, cube);

	// Added for The Wall: Init the portals
	
	// Back portal
	if (0) // Skip - it causes problems for this case
	addPortal(thisCell,	// Add to this cell
		southCell,			// Destination cell
		vec3(3, 0, 10), // Four corners of portal. Must be a square.
		vec3(-3, 0, 10),
		vec3(3, 3, 10),
		vec3(-3, 3, 10),
		T(0,0,-10), // Translation part of portal transformation
		IdentityMatrix()); // Rotation part of portal transformation
	
	// Right portal
	addPortal(thisCell,	// Add to this cell
		westCell,			// Destination cell
		vec3(5, 0, 2), // Four corners of portal. Must be a square.
		vec3(5, 0, 8),
		vec3(5, 3, 2),
		vec3(5, 3, 8),
		T(-5,0,15), // Translation part of portal transformation
		Ry(M_PI/2)); // Rotation part of portal transformation
}



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
	texShader = loadShadersG("textured.vert", "textured.frag", "textured.gs");
	portalShader = loadShaders("portal.vert", "portal.frag");
	printError("init shader");
	
	// Upload geometry to the GPU:
	cube = LoadModel("cube.obj");
//	halfcube = LoadModel("halfcube.obj"); Should really be used

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
	LoadTGATextureSimple("bluegrass.tga", &bluegrasstex);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, rocktex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);
	
	// Build the cells.
	
	addCell(2);
		
	buildStraightCell(0, 1, 1, grasstex);
	buildCornerCell(  1, 0, 0, woodtex);

	currentCell = 0;


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
	for (int i = 0; i < cells[currentCell].cubeCount; i++) // for every cube
	{
		vec3 pp = ShortestAxis(cells[currentCell].cubes[i].minp, cells[currentCell].cubes[i].maxp, campos, &d, kCamSize);
		if (d < 1000000)
		{
			mindist = d;
			if (pp.x != close.x || pp.z != close.z)
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
	
	for (int i = 0; i < cells[currentCell].portalCount; i++)
		{
		vec3 a = cells[currentCell].portals[i].a;
		vec3 b = cells[currentCell].portals[i].b;
		vec3 c = cells[currentCell].portals[i].c;
		vec3 d = cells[currentCell].portals[i].d;
		// Test assumed portals are axis aligned!
		if ((a.x == b.x) && (b.x == c.x) && (c.x == d.x)) // Portal along z
		{
			if ((a.x < campos.x) != (a.x < oldpos.x))
			// Must also check borders
			if (((a.z - campos.z)*(campos.z - b.z)) >= 0)
			if (((a.y - campos.y)*(campos.y - c.y)) >= 0)
			{
				// Passed through!
				// campos is not guaranteed to be in the center, but rather a bit beyond!
				// Needs to be transformed to end up inside portal.
				
				campos = campos - cells[currentCell].portals[i].center; // Rotate around portal
				campos = cells[currentCell].portals[i].rot * campos;
				campos = campos + cells[currentCell].portals[i].center;
														// Then translate to portal destination
				campos = cells[currentCell].portals[i].trans * campos;
				forward = cells[currentCell].portals[i].rot * forward; // Also rotate rotation!
				
				printf("SWITCH X %d to %d\n", currentCell, cells[currentCell].portals[i].dest);
				currentCell = cells[currentCell].portals[i].dest;
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
				campos = campos - cells[currentCell].portals[i].center; // Rotate around portal
				campos = cells[currentCell].portals[i].rot * campos;
				campos = campos + cells[currentCell].portals[i].center;
														// Then translate to portal destination
				campos = cells[currentCell].portals[i].trans * campos;
				forward = cells[currentCell].portals[i].rot * forward; // Also rotate rotation!

				printf("SWITCH Z %d to %d\n", currentCell, cells[currentCell].portals[i].dest);

				currentCell = cells[currentCell].portals[i].dest;
				// Passed through!
			}
		}
// Y skipped, we don't expect portals in floor or ceilings - yet
	}

	
}

void DrawCell(int currentCell, int fromCell, mat4 worldToView, mat4 modelToWorld, int count) // Draw cells recursively! Needed for the semitransparent portal!
{
	mat4 m;
	
	if (count < 2)
	for (int i = 0; i < cells[currentCell].portalCount; i++)
	{
		{
			vec3 c2 = cells[currentCell].portals[i].center; // Current portal center
			c2 = cells[currentCell].portals[i].trans * c2; // Translate to next
			
			mat4 tm = T(-c2.x, -c2.y, -c2.z); // Translation portal destination to origin
			mat4 m = inverse(cells[currentCell].portals[i].rot) * tm; // Apply rotation
			
			// Then translate to center
			vec3 c1 = cells[currentCell].portals[i].center; // Nuvarande portal
			mat4 tf = T(c1.x, c1.y, c1.z); // Translation portal destination to current
			m = tf * m;

			DrawCell(cells[currentCell].portals[i].dest, currentCell, worldToView, Mult(modelToWorld, m), count+1);
		}
	}
		
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	for (int i = 0; i < cells[currentCell].cubeCount; i++)
	{
		// Cube
		glUseProgram(texShader);
		glBindTexture(GL_TEXTURE_2D, cells[currentCell].cubes[i].tex);
		m = worldToView * modelToWorld * T(cells[currentCell].cubes[i].pos.x,cells[currentCell].cubes[i].pos.y,cells[currentCell].cubes[i].pos.z);
		m = m * S(cells[currentCell].cubes[i].scale.x,cells[currentCell].cubes[i].scale.y,cells[currentCell].cubes[i].scale.z);
		glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
		glUniform3fv(glGetUniformLocation(texShader, "scaling"), 1, (GLfloat *)&cells[currentCell].cubes[i].scale);
		DrawModel(cells[currentCell].cubes[i].model, texShader, "inPosition", "inNormal", "inTexCoord");
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glUseProgram(portalShader);
	glUniformMatrix4fv(glGetUniformLocation(portalShader, "modelviewMatrix"), 1, GL_TRUE, Mult(worldToView, modelToWorld).m);
	for (int i = 0; i < cells[currentCell].portalCount; i++)
	{
		DrawModel(cells[currentCell].portals[i].model, portalShader, "inPosition", NULL, "inTexCoord");
	}

}

void display(void)
{
	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	mat4 worldToView, m;
	
	Move();

	worldToView = lookAtv(campos, campos + forward, up);
	
	a += 0.1;

	DrawCell(currentCell, -1, worldToView, IdentityMatrix(), 0); // Draw cells recursively! Needed for the semitransparent portal!
	
	printError("display");
	
	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(800,800);
	glutCreateWindow ("3. Multiple rooms");
	glutRepeatingTimer(20);
	glutDisplayFunc(display); 
	glutKeyboardFunc(keys);
	init ();
	glutMainLoop();
}
