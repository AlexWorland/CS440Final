/* 
	Alek Michael
	Conway's Game of Life in 2D. This is meant as a introduction to me about the game and how it would be made in GLUT.
	Much thanks to Coding train for an awesome explanation and run down of the game. I highly reccomend that channel!
	Ideas to expand this would to use the mouse for initial settings and perhaps allow the user select initial placements.
	Mouse clicks could also determine when to iterate the states.
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
int main (int argc, char** argv)
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
const int SCREEN_WIDTH = 1200, SCREEN_HEIGHT = 800, DEPTH = 1200, RESOLUTION = 80,  FPS = 10; // area of screen, size of rectangles, and FPS
const int COLUMNS = SCREEN_WIDTH / RESOLUTION, ROWS = SCREEN_HEIGHT / RESOLUTION, AISLES = DEPTH / RESOLUTION; // # of columns, rows, and aisles in grid
int GRID[COLUMNS][ROWS][AISLES]; // Current (columns x rows x aisles) grid
int*** nextState = NULL; // Grid after iteration
float cameraAngle = 1;
int startingAmount = 40;
GLfloat mouseX;
GLfloat mouseY;

// ********* INIT GLUT *********
void initGlut(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glEnable(GLUT_MULTISAMPLE);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	//glutInitWindowPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
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

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, SCREEN_WIDTH, -1, SCREEN_HEIGHT, 10000, -10000);
		//gluPerspective(30, SCREEN_WIDTH / SCREEN_HEIGHT, 0.1, 1000);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		gluLookAt(-20, -50, 180, 0.5, 0, 0, 0, 1, 0);
		//glTranslatef(0, 0, -350);
		glRotatef(25, 0, 1, 0);

		glutWireCube(SCREEN_WIDTH);

		for (int i = 0; i < COLUMNS; i++)
			for (int j = 0; j < ROWS; j++)
				for (int k = 0; k < AISLES; k++)
				{
					int x = i * RESOLUTION, y = j * RESOLUTION, z = k * RESOLUTION; // get where the rect starts
					
					//glBegin(GL_POLYGON); // draw rects

					// determine color (black is alive, white is dead)
					if (GRID[i][j][k] != 1) continue;

					//glColor3f(0, 0, 0);
					glColor3f(1.0 / (i + 2), 1.0 / (j + 2), 1.0 / (k + 2));
					
					// draw rect, sub by 1 for spacing between cells for better visibility
					//glVertex3i(x, y, z);
					//glVertex3i(x + RESOLUTION - 1, y, z);
					//glVertex3i(x + RESOLUTION - 1, y + RESOLUTION - 1, (z));
					//glVertex3i(x, y + RESOLUTION - 1, (z));

					//glEnd();
					glPushMatrix();
					glTranslatef(x, y, z - 350);
					glScalef(0.8, 0.8, 0.8);
					glutSolidCube(RESOLUTION);
					glColor3f(0.5, 0.5, 0.5);
					glutWireCube(RESOLUTION);
					glPopMatrix();
				}

		//glEnd();

		// render
		glFlush();

		glutSwapBuffers();
}

// ********* INIT SETTINGS AND GRID *********
void Init()
{
    glClearColor(1.0,1.0,1.0,0.0);
    glColor3f(0.0f,0.0f,0.0f);
    glPointSize(4.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluOrtho2D(1.0,SCREEN_WIDTH,0.0,SCREEN_HEIGHT);
	//glOrtho(1.0, SCREEN_WIDTH, 0.0, SCREEN_HEIGHT, 0.0, SCREEN_WIDTH);
	//glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glEnable(GL_DEPTH_TEST); // enable depth test

	// fill grid with random points
	while (startingAmount > 0)
		for (int i = 0; i < COLUMNS; i++)
			for (int j = 0; j < ROWS; j++)
				for (int k = 0; k < AISLES; k++)
				{
					if (GRID[i][j][k] == 1) continue;
					int decision = rand() % 2;
					if (decision == 1) startingAmount--;
					GRID[i][j][k] = (startingAmount <= 0) ? 0 : decision;
				}
}

// ********* NEXT STATE LOGIC *********
void getNextState()
{
	// fill nextState with the next iteration of grid, then set grid to nextState
	nextState = new int** [COLUMNS];
	
	for (int i = 0; i < COLUMNS; i++)
	{
		nextState[i] = new int* [ROWS];
		for (int j = 0; j < ROWS; j++)
			nextState[i][j] = new int[AISLES];
	}

	

	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
			{
				// get current state
				int currState = GRID[i][j][k];

				// find # of neighbors around this cell
				int neighborCount = countNeighbors(i, j, k);
				
				/*
					Conway's rules in 3d can be many things. We will use the rules:
					An alive cell with 4 or 5 neighbors lives on.
					If the cell is dead but has exactly 5 neighbors, it will be alive.
					Otherwise, the cell dies.
				*/
				if ((currState == 1 && (neighborCount >= 4 && neighborCount <= 5)) || (currState == 0 && neighborCount == 5))
					nextState[i][j][k] = 1;
				else
					nextState[i][j][k] = 0;
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
} 

// ********* # OF NEIGHBORS LOGIC FOR POINT (x,y) *********
int countNeighbors(int x, int y, int z)
{
	int count = 0;

	// Check each direction from the point (x, y)
	// This method wraps edges.
	for (int i = -1; i < 2; i++)
		for (int j = -1; j < 2; j++)
			for (int k = -1; k < 2; k++)
			{
				/*
					Imagine we have a 10x10 grid. 
					If we are looking at a position of x = 9:
					We always want to know what is the to right of that position, but x + 1 = 10 which is out of bounds.
					BUT, x + 1 + 10 (columns) % 10 = 20 % 10 = 0
					Therefore, anytime we are at an edge it will wrap around when we look towards the right.

					Same logic for other directions.
					This is a little hard to understand IMO, but I hope this explanation is good enough.
				*/
				int column = (x + i + COLUMNS) % COLUMNS;
				int row = (y + j + ROWS) % ROWS;
				int aisle = (z + k + AISLES) % AISLES;
				count += GRID[column][row][aisle]; // Count alive cells 
			}

	count -= GRID[x][y][z]; // dont include self
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
        case 'q':
            exit(0);
            break;
        case 'a':
            translate(SCREEN_WIDTH / 10, 0, 0);
            break;
        case 'd':
            translate(-SCREEN_WIDTH / 10, 0, 0);
            break;
        case 'w':
            translate(0, 0.1, 0);
            break;
        case 's':
            translate(0, -0.1, 0);
            break;
        case 'z':
            rotate(1, 0, 0, 1);
            break;
        case 'x':
            rotate(-1, 0, 0, 1);
            break;
        case ' ':
            // pause the game
            break;
        default:
            break;   
    }
}

// ********* INPUT CALLBACKS **************
void mouse(int button, int state, int x, int y) {
	std::cout << "Mouse event: " << button << " " << state << " " << std::endl;
    if (button == 3) { // scroll wheel up
        zoom(5);
    } else if (button == 4) { // scroll wheel down
        zoom(-5);
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouseX = x;
        mouseY = y;
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        mouseX = x;
        mouseY = y;
    } else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
        mouseX = x;
        mouseY = y;
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        mouseX = x;
        mouseY = y;
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
        mouseX = x;
        mouseY = y;
    } else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP) {
        mouseX = x;
        mouseY = y;
    }
}