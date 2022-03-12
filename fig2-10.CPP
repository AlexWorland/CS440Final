/*
	CS440 Final Project Part 1
	Group 2
	Conway's Game of Life in the Third Dimension

	3D implementation to John Conway's game of life using GLUT.
*/

/*
	Controls:
		q
			Quits the program
		w/a/s/d
			Camera controls
		r/f
			Zoom in and out
		z/x
			Rotate horizontally
		p
			Start and pause game
		t
			Reset game to setting mode
		l
			Reset game with random seed

		In setting mode:
		Arrow keys
			Move cursor in x/y axes
		Shift + up/down
			move cursor in z axis
		Spacebar
			Select and unselect a cell for the initial seed
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
	#include "glut.h"
	#include "math.h"
	#include <cctype>
#endif

// ********* PROTOTYPES *********
void Display(void);
void Init(void);
void initGlut(int argc, char** argv);
void setSeed(void);
void createRandomState(void);
void getNextState(void);
int countNeighbors(int x, int y, int z);
void update(int value);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void arrowKeyInput(int key, int x, int y);
void chooseCursorPosition(void);
// ***** TRANSFORM FUNCTIONS *****
void translate(float x, float y, float z);
void rotate(float angle, float x, float y, float z);
void zoom(float factor);

/*
	The State of each cell in the game of life is either alive (life) or dead (death).
	The other states are for the setting phase to tell the user where their cursor is and what they've selected.  
*/
enum State
{
	cursorOnSet = 4, // Cursor on a set cell
	userCursor = 3, // Cursor off a set cell
	userSet = 2, // A set cell with no cursor on top
	life = 1, // A life cell in simulation 
	death = 0 // A dead cell in simulation 
};

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
const int SCREEN_WIDTH = 1200 * scale, SCREEN_HEIGHT = 800 * scale, DEPTH = 1200 * scale, RESOLUTION = 160; // dimensions (x,y,z) of grid, size of blocks, and FPS
const int COLUMNS = SCREEN_WIDTH / RESOLUTION, ROWS = SCREEN_HEIGHT / RESOLUTION, AISLES = DEPTH / RESOLUTION; // # of columns, rows, and aisles in grid
int FPS = 10;
State GRID[COLUMNS][ROWS][AISLES]; // Current (columns x rows x aisles) grid, holds states of cells
State*** nextState = NULL; // next generation grid
int startingAmount = 10000; // initial cell count
bool isPaused = false, isSetting = true; // pause flag 
int userCursorX = COLUMNS / 2, userCursorY = ROWS / 2, userCursorZ = AISLES / 2;
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
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(Display);
	glutSpecialFunc(arrowKeyInput);
}

// ********* DISPLAY FUNC *********
void Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear screen
	bool allDead = true;
	// draw grid
	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
			{
				int x = i * RESOLUTION, y = j * RESOLUTION, z = k * RESOLUTION; // get where the rect starts

				// If the cell is dead, move on 
				if (GRID[i][j][k] == death) continue;
				
				allDead = false;

				glPushMatrix();
				glTranslatef(x, y, z - 350);
				glScalef(0.8, 0.8, 0.8);

				// draw cube

				// if the game started, set the cube to its color
				if (!isSetting && (GRID[i][j][k] == userSet || GRID[i][j][k] == cursorOnSet))
					GRID[i][j][k] = life;

				if (GRID[i][j][k] == life)
					glColor3f(0, 0, 0);
				else if (GRID[i][j][k] == userSet) //cell set by user
					glColor3f(0, 0, 1.0);
				else if (GRID[i][j][k] == cursorOnSet)
					glColor3f(0, 1.0, 0);

				glutSolidCube(RESOLUTION);

				// draw outline along cube
				glColor3f(0.5, 0.5, 0.5);
				glutWireCube(RESOLUTION);

				glPopMatrix();
			}

	// If everyone dies, restart the game
	if (allDead) 
	{
		userCursorX = COLUMNS / 2;
		userCursorY = ROWS / 2;
		userCursorZ = AISLES / 2;
		isSetting = true;
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
	glOrtho(-1, SCREEN_WIDTH, -1, SCREEN_HEIGHT, 100000, -100000);

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
				GRID[i][j][k] = death;

	// assign random states to each cell (alive or dead)
	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
			{	
				int decision = rand() % 200;

				if (decision > 175)
					GRID[i][j][k] = life;
				else 
					GRID[i][j][k] = death;
			}
}

// ********* SET SEED *********
void setSeed()
{
	// don't need to do this if we are selecting an already set cell 
	if (GRID[userCursorX][userCursorY][userCursorZ] == cursorOnSet)
		return; 

	// remove the previous cursor
	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
				if (GRID[i][j][k] == cursorOnSet)
					GRID[i][j][k] = userSet;
				else if (GRID[i][j][k] != userSet)
					GRID[i][j][k] = death;
	
	// set on or off the cell
	if (GRID[userCursorX][userCursorY][userCursorZ] == userSet)
		GRID[userCursorX][userCursorY][userCursorZ] = cursorOnSet;
	else 
		GRID[userCursorX][userCursorY][userCursorZ] = userCursor;
}
// ********* SET SEED LOGIC *********
void chooseCursorPosition()
{
	// if we aren't selecting, don't do this
	if (!isSetting) return; 

	// reverse the choice
	if (GRID[userCursorX][userCursorY][userCursorZ] == userSet || GRID[userCursorX][userCursorY][userCursorZ] == cursorOnSet)
		GRID[userCursorX][userCursorY][userCursorZ] = death;
	else
		GRID[userCursorX][userCursorY][userCursorZ] = userSet;
}

// ********* NEXT STATE LOGIC *********
void getNextState()
{
	if (isPaused) 
		return;
	

	// fill nextState with the next iteration of grid, then set grid to nextState
	nextState = new State** [COLUMNS];

	// fill with empty arrays 
	for (int i = 0; i < COLUMNS; i++)
	{
		nextState[i] = new State* [ROWS];
		for (int j = 0; j < ROWS; j++)
			nextState[i][j] = new State[AISLES];
	}

	// get next state 
	for (int i = 0; i < COLUMNS; i++)
		for (int j = 0; j < ROWS; j++)
			for (int k = 0; k < AISLES; k++)
			{
				// get current state
				State currState = GRID[i][j][k];

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
	if (isSetting)
		setSeed(); // display setting screen phase
	else
		getNextState(); // when done selecting, calculate the next state 

	glutPostRedisplay(); // Render the next, now current, grid

	glutTimerFunc(1000 / ((isSetting) ? FPS : 5), update, 0); // Wait to update again
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
		rotate(1, 0, 1, 0);
		break;
	case 'x':
		rotate(-1, 0, 1, 0);
		break;
	// pause/start the game
	case 'p':
		if (isSetting)
			isSetting = false; // start the game
		else 
			isPaused = !isPaused;		
		break;
	// Reset grid to selection 
	case 't':
		if (isSetting == true)
			// set grid to nothing
			for (int i = 0; i < COLUMNS; i++)
				for (int j = 0; j < ROWS; j++)
					for (int k = 0; k < AISLES; k++)
							GRID[i][j][k] = death;
		isSetting = true;
		break;
	// Select cell
	case ' ':
		chooseCursorPosition();
		break;
	// reset game
	case 'l':
		isSetting = false;
		createRandomState();
		break;
	default:
		break;
	}
}

// ********* INPUT CALLBACKS **************
void arrowKeyInput(int key, int x, int y)
{
	int keyboardFunctionKey = glutGetModifiers();
	
	switch (key)
	{
	// up arrow key moves up both in the y axis and z
	case GLUT_KEY_UP:
		if (keyboardFunctionKey == 1) // hold shift + up to go up 
			if (userCursorZ + 1 < AISLES)
				userCursorZ++;
			else
				userCursorZ = 0;
		else 
			if (userCursorY + 1 < ROWS)
				userCursorY++;
			else
				userCursorY = 0;
			break;
	// down arrow key moves down in both the y and z axis
	case GLUT_KEY_DOWN:
		if (keyboardFunctionKey == 1) //shift + down to go down
			if (userCursorZ - 1 >= 0)
				userCursorZ--;
			else
				userCursorZ = AISLES - 1;
		else 
			if (userCursorY - 1 >= 0)
				userCursorY--;
			else
				userCursorY = ROWS - 1;
			break;
	// move right and left with respective arrow keys in x axis
	case GLUT_KEY_RIGHT:
		if (userCursorX + 1 < COLUMNS)
			userCursorX++;
		else
			userCursorX = 0;
		break;
	case GLUT_KEY_LEFT:
		if (userCursorX - 1 >= 0)
			userCursorX--;
		else
			userCursorY = COLUMNS - 1;
		break;
	default:
		break;
	}
}