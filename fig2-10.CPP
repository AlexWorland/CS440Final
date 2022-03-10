/*
	CS440 Final Project Part 1
	Group 2
	Conway's Game of Life in the Third Dimension

	3D implementation to John Conway's game of life using GLUT.
*/

#include <iostream>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <math.h>
#include <cctype>
#else
#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
//#include <iostream.h>
#include "glut.h"
#include "math.h"
#include <cctype>
#endif

// ********* PROTOTYPES *********
void Display(void);
void Init(void);
void initGlut(int argc, char** argv);
void createRandomState(void);
void getNextState(void);
int countNeighbors(int x, int y, int z);
void update(int value);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
// ***** TRANSFORM FUNCTIONS *****
void translate(float x, float y, float z);
void rotate(float angle, float x, float y, float z);
void zoom(float factor);



// ********* MAIN *********
int main(int argc, char** argv)
{
	// initialize GLUT settings
	initGlut(argc, argv);

	// initialize the first grid and 
	Init();

	// update the game and display the results
	update(0);
	glutMainLoop();

	return EXIT_SUCCESS;
}

// ********* GLOBALS *********
const int scale = 5; // grid scale 
const int SCREEN_WIDTH = 1200 * scale, SCREEN_HEIGHT = 800 * scale, DEPTH = 1200 * scale, RESOLUTION = 80, FPS = 10; // dimensions (x,y,z) of grid, size of blocks, and FPS
const int COLUMNS = SCREEN_WIDTH / RESOLUTION, ROWS = SCREEN_HEIGHT / RESOLUTION, AISLES = DEPTH / RESOLUTION; // # of columns, rows, and aisles in grid
int GRID[COLUMNS][ROWS][AISLES]; // Current (columns x rows x aisles) grid, holds states of cells
int*** nextState = NULL; // next generation grid
float cameraAngle = 1; // angle of camera
int startingAmount = 20000; // initial cell count
bool isPaused = false; // pause flag 
GLfloat mouseX, mouseY; // mouse coordinates

// ********* INIT GLUT *********
void initGlut(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glEnable(GLUT_MULTISAMPLE); // enable AA
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Conway's Game of Life 3D");
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(Display);
}

// ********* DISPLAY FUNC *********
void Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear screen

	// draw grid
	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
			{
				int x = i * RESOLUTION, y = j * RESOLUTION, z = k * RESOLUTION; // get where the rect starts


				// If the cell is dead, move on 
				if (GRID[i][j][k] != 1) continue;

				glPushMatrix();

				glTranslatef(x, y, z - 350);
				glScalef(0.8, 0.8, 0.8);

				// draw cube
				glColor3f(1.0 / (i + 2), 1.0 / (j + 2), 1.0 / (k + 2));
				glutSolidCube(RESOLUTION);

				// draw outline along cube
				glColor3f(0.5, 0.5, 0.5);
				glutWireCube(RESOLUTION);

				glPopMatrix();
			}

	// commence 
	glFlush();

	glutSwapBuffers();
}

// ********* INIT SETTINGS AND GRID *********
void Init()
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glColor3f(0.0f, 0.0f, 0.0f);
	glPointSize(4.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST); // enable depth test

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, SCREEN_WIDTH, -1, SCREEN_HEIGHT, 10000, -10000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//get proper angle of grid 
	gluLookAt(-20, -50, 180, 0.5, 0, 0, 0, 1, 0);
	glRotatef(25, 0, 1, 0);

	// create a random grid 
	createRandomState();
}

// ********* RANDOM GRID LOGIC *********
void createRandomState()
{
	// set grid to nothing
	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
				GRID[i][j][k] = 0;

	// fill random spots of the grid until we have reached the limit startingAmount
	int temp = startingAmount;
	while (temp > 0)
		for (int i = 0; i < COLUMNS; i++)
			for (int j = 0; j < ROWS; j++)
				for (int k = 0; k < AISLES; k++)
				{
					if (temp <= 0) break;
					if (GRID[i][j][k] == 1) continue;
					int decision = rand() % 200;

					if (decision > 180)
					{
						GRID[i][j][k] = 1;
						temp--;
					}
					else GRID[i][j][k] = 0;
				}
}

// A cell has two states, alive or dead 
enum State
{
	life = 1,
	death = 0
};

// ********* NEXT STATE LOGIC *********
void getNextState()
{
	if (isPaused) {
		return;
	}

	// fill nextState with the next iteration of grid, then set grid to nextState
	nextState = new int** [COLUMNS];

	// fill with empty arrays 
	for (int i = 0; i < COLUMNS; i++)
	{
		nextState[i] = new int* [ROWS];
		for (int j = 0; j < ROWS; j++)
			nextState[i][j] = new int[AISLES];
	}

	// get next state 
	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
			{
				// get current state
				int currState = GRID[i][j][k];

				// find # of neighbors around this cell
				int neighborCount = countNeighbors(i, j, k);

				/*
					There are no agreed upon rules to a 3D implementation for this game.
					We chose our ruleset based on what appeared most life like.
					The rules that are used are:
						A cell can only be alive in the next generation if:
							* It is already alive AND has 5-7 live neighbors.
							* It is dead but has exactly 6 live neighbors.
						Otherwise, the cell is dead in the next iteration.
				*/

				if ((currState == life && (neighborCount >= 5 && neighborCount <= 7)) || (currState == death && neighborCount == 6))
					nextState[i][j][k] = life;
				else
					nextState[i][j][k] = death;
			}

	// Fill grid with the next state
	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
				GRID[i][j][k] = nextState[i][j][k];

	// Free memory
	for (int i = 0; i < COLUMNS; i++)
	{
		for (int j = 0; j < ROWS; j++)
			delete[] nextState[i][j];
		delete[] nextState[i];
	}
	delete[] nextState;

	isPaused = true;
}

// ********* # OF NEIGHBORS LOGIC FOR POINT (x,y,z) *********
int countNeighbors(int x, int y, int z)
{
	// neighbor count
	int count = 0;

	// Check each direction from the point (x, y, z)
	// This method wraps edges.
	for (int i = -1; i < 2; i++)
		for (int j = -1; j < 2; j++)
			for (int k = -1; k < 2; k++)
			{
				// Using modulus allows for the edges to wrap around, keeping the grid from freezing up.
				int column = (x + i + COLUMNS) % COLUMNS;
				int row = (y + j + ROWS) % ROWS;
				int aisle = (z + k + AISLES) % AISLES;
				count += GRID[column][row][aisle]; // Count alive cells 
			}

	count -= GRID[x][y][z]; // dont include self as a neighbor 
	return count;
}

// ********* UPDATE *********
void update(int value) {
	getNextState(); // Calculate the next state 

	cameraAngle += .2;

	glutPostRedisplay(); // Render the next, now current, grid

	glutTimerFunc(1000 / FPS, update, 0); // Wait to update again
}


// ********* SET VIEWPORT AND WINDOW *********

// ***** TRANSFORM FUNCTIONS *****
void translate(float x, float y, float z)
{
	glTranslatef(x, y, z);
}
void rotate(float angle, float x, float y, float z)
{
	glRotatef(angle, x, y, z);
}
void zoom(float factor)
{
	glScalef(factor, factor, factor);
}


// ********* KEYBOARD FUNCTION *********
void keyboard(unsigned char key, int x, int y)
{
	key = tolower(key);
	std::cout << key << std::endl;
	switch (key)
	{
		// quit
	case 'q':
		exit(0);
		break;
		// move camera
	case 'a':
		translate(SCREEN_WIDTH / 10, 0, 0);
		break;
	case 'd':
		translate(-SCREEN_WIDTH / 10, 0, 0);
		break;
	case 's':
		translate(0, SCREEN_HEIGHT / 10, 0);
		break;
	case 'w':
		translate(0, -SCREEN_HEIGHT / 10, 0);
		break;
		// zoom in / out
	case 'r':
		zoom(1.1);
		break;
	case 'f':
		zoom(0.9);
		break;
		// rotate grid
	case 'z':
		rotate(1, 0, 0, 1);
		break;
	case 'x':
		rotate(-1, 0, 0, 1);
		break;
		// pause the game
	case ' ':
		isPaused = !isPaused;
		break;
		// reset game
	case 'l':
		createRandomState();
		break;
	default:
		break;
	}
}

// ********* INPUT CALLBACKS **************
void mouse(int button, int state, int x, int y) {
	// get mouse coordinates on screen
	std::cout << "Mouse event: " << button << " " << state << " " << std::endl;
	if (button == 3) { // scroll wheel up not working :(
		zoom(5);
	}
	else if (button == 4) { // scroll wheel down not working :(
		zoom(-5);
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mouseX = x;
		mouseY = y;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		mouseX = x;
		mouseY = y;
	}
	else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
		mouseX = x;
		mouseY = y;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		mouseX = x;
		mouseY = y;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		mouseX = x;
		mouseY = y;
	}
	else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP) {
		mouseX = x;
		mouseY = y;
	}
}