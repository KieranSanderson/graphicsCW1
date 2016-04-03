
//includes areas for keyboard control, mouse control, resizing the window
//and draws a spinning rectangle
#include <windows.h>		// Header File For Windows
//#include <gl\gl.h>			// Header File For The OpenGL32 Library
//#include <gl\glu.h>			// Header File For The GLu32 Library
/*	nvImage.h contains all of the function prototypes needed for image loading.
nvImage.h includes glew.h and so we don't need to include gl and glu header files for OpenGL
*/
#include "Image_Loading/nvImage.h"

int		mouse_x=0,			mouse_y=0;
int screenWidth=480,   screenHeight=480;
bool keys[256];

//room variables
GLuint tileTexture		= 0;
GLuint cornerBricks		= 0;
GLuint wallBricks		= 0;
GLuint doorBricks		= 0;
GLuint chaTextureGrid	= 0;
float wallWidth			= 2;
bool paused				= false;
bool lDoor				= false;
bool bDoor				= false;
bool rDoor				= true;


//variables for character
float chaSize			= 1.5;					//height and width of character
float chaSpeed			= 0.30;
float cTexBegX			= 0.0;
float cTexEndX			= cTexBegX + 0.2;
float cTexBegY			= 0.8;
float cTexEndY			= cTexBegY + 0.2;
bool attacking			= false;
bool firing				= false;
bool tface				= true;					//booleans for facing
bool lface				= false;
bool bface				= false;
bool rface				= false;
float Xcha				= 0.0;
float Ycha				= (chaSize + wallWidth - 20.00);			//x and y coords of the character

//variables for boss
float Xboss				= 0.00;
float Yboss				= 14.50;
float chaseSpeed		= 0.02;
float bossWidth			= 3.50;
float bossHeight		= 3.50;
bool  chase				= true;					//boolean to move boss toward character
float maxSpeed			= 0.0135;

//OPENGL FUNCTION PROTOTYPES
void display();									//draws everything to the screen
void loadBoss();								//draws boss poly
void loadCharacter();							//draws character poly
void loadSword(int direction);					//loads attack based on direction
void loadWalls(float cCoordx, float cCoordy);	//loads in walls and corners
void changeFacing(int direction);				//changes facing boolean
void bossChase();								//processes boss actions
void reshape(int width, int height);			//called when the window is resized
void init();									//called in winmain when the program starts.
void processKeys();								//called in winmain to process keyboard controls


GLuint loadPNG(char* name)
{
	// Texture loading object
	nv::Image img;

	GLuint textureID;

	// Return true on success
	if (img.loadImageFromFile(name))
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, img.getInternalFormat(), img.getWidth(), img.getHeight(), 0, img.getFormat(), img.getType(), img.getLevel(0));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
	}

	else
		MessageBox(NULL, "Failed to load texture", "End of the world", MB_OK | MB_ICONINFORMATION);

	return textureID;
}

void display()									
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	glLoadIdentity();

	loadWalls(0,0);

	//draw axes
	glLineWidth(3.0);
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_LINES);
		glVertex2f(-20,  0);	glVertex2f( 20,  0);
		glVertex2f(  0, 20);	glVertex2f(  0,-20);
	glEnd();
	glLineWidth(0.2);

	//Boss and character layering
	if (Yboss > Ycha)
	{
		loadBoss();
		loadCharacter();
	} 
	else
	{
		loadCharacter();
		loadBoss();
	}

	glFlush();
}

void loadBoss()
{
	if (chase) { bossChase(); }
	glPushMatrix();
	glTranslatef(Xboss, Yboss, 0.0);
		glColor3f(1.0,0.0,0.0);
		glBegin(GL_POLYGON);
			glVertex2f(-bossWidth,-bossHeight);
			glVertex2f(-bossWidth, bossHeight);
			glVertex2f(bossWidth, bossHeight);
			glVertex2f(bossWidth,-bossHeight);
		glEnd();
	glPopMatrix();
}

void loadCharacter()
{
	glPushMatrix();
	glTranslatef(Xcha, Ycha, 0.0);
		if (tface && attacking) { loadSword(1); }
		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0,1.0,1.0,1.0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, chaTextureGrid);
		glBegin(GL_POLYGON);
			glTexCoord2f(cTexBegX, cTexBegY);  glVertex2f(-chaSize,-chaSize);
			glTexCoord2f(cTexEndX, cTexBegY);  glVertex2f( chaSize,-chaSize);
			glTexCoord2f(cTexEndX, cTexEndY);  glVertex2f( chaSize, chaSize);
			glTexCoord2f(cTexBegX, cTexEndY);  glVertex2f(-chaSize, chaSize);
		glEnd();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		if (bface && attacking) { loadSword(2); }
		if (lface && attacking) { loadSword(3); }
		if (rface && attacking) { loadSword(4); }
	glPopMatrix();
}

void reshape(int width, int height)						// Resize the OpenGL window
{
	screenWidth=width; screenHeight = height;           // to ensure the mouse coordinates match 
														// we will use these values to set the coordinate system

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	gluOrtho2D(-20,20,-20,20);							// set the coordinate system for the window

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

void init()
{
	glClearColor(0.0,0.0,0.0,0.0);						//sets the clear colour to black
														//glClear(GL_COLOR_BUFFER_BIT) in the display function
														//will clear the buffer to this colour.

	// Texture loading


	tileTexture = loadPNG("floortiles.png");
	wallBricks = loadPNG("wallBricks.png");
	cornerBricks = loadPNG("cornerBricks.png");
	doorBricks = loadPNG("doorBricks.png");
	chaTextureGrid = loadPNG("chaTextureGrid.png");
}

void loadWalls(float cCoordx, float cCoordy)
{
	glLoadIdentity();

	//floor
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, tileTexture);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0, 0.0); glVertex2f(cCoordx - 20.0, cCoordy - 20.0);
			glTexCoord2f(0.0, 1.0); glVertex2f(cCoordx - 20.0, cCoordy + 20.0);
			glTexCoord2f(1.0, 1.0); glVertex2f(cCoordx + 20.0, cCoordy + 20.0);
			glTexCoord2f(1.0, 0.0); glVertex2f(cCoordx + 20.0, cCoordy - 20.0);
		glEnd();
	glPopMatrix();

	//walls

	//left wall
	//boolean leftDoor to be made
	//if false, just a wall, if true, has a gap that is a door
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, wallBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx - 20.0,				cCoordy - 20.0 + wallWidth);	//bottom left
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy - 20.0 + wallWidth);	//bottom right
			glTexCoord2f(1.0,  8.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy - wallWidth);			//top right
			glTexCoord2f(0.0,  8.0);  glVertex2f(cCoordx - 20.0,				cCoordy - wallWidth);			//top left
		glEnd();
	glPopMatrix();
	
	//Door, boolean for closed or open to be made
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, wallBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx - 20.0,				cCoordy - wallWidth);		//bottom left
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy - wallWidth);		//bottom right
			glTexCoord2f(1.0,  2.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy + wallWidth);		//top right
			glTexCoord2f(0.0,  2.0);  glVertex2f(cCoordx - 20.0,				cCoordy + wallWidth);		//top left
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, wallBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx - 20.0,				cCoordy + wallWidth);			//bottom left
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy + wallWidth);			//bottom right
			glTexCoord2f(1.0,  8.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy + 20.0 - wallWidth);	//top right
			glTexCoord2f(0.0,  8.0);  glVertex2f(cCoordx - 20.0,				cCoordy + 20.0 - wallWidth);	//top left
		glEnd();
	glPopMatrix();
	//End left wall


	//right wall
	//boolean rightDoor to be made
	//if false, just a wall, if true, has a gap that is a door
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, wallBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy + wallWidth);			//bottom left
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx + 20.0,				cCoordy + wallWidth);			//bottom right
			glTexCoord2f(0.0,  8.0);  glVertex2f(cCoordx + 20.0,				cCoordy + 20.0 - wallWidth);	//top right
			glTexCoord2f(1.0,  8.0);  glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy + 20.0 - wallWidth);	//top left
		glEnd();
	glPopMatrix();
	
	//Door, boolean for closed or open to be made
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, doorBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(1.0, (2.0 / 3.0));	glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy - wallWidth);		//bottom left
			glTexCoord2f(1.0, 1.0);			glVertex2f(cCoordx + 20.0,				cCoordy - wallWidth);		//bottom right
			glTexCoord2f(0.0, 1.0); 		glVertex2f(cCoordx + 20.0,				cCoordy + wallWidth);		//top right
			glTexCoord2f(0.0, (2.0 / 3.0)); glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy + wallWidth);		//top left
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, wallBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy - 20.0 + wallWidth);	//bottom left
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx + 20.0,				cCoordy - 20.0 + wallWidth);	//bottom right
			glTexCoord2f(0.0,  8.0);  glVertex2f(cCoordx + 20.0,				cCoordy - wallWidth);			//top right
			glTexCoord2f(1.0,  8.0);  glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy - wallWidth);			//top left
		glEnd();
	glPopMatrix();
	//End right wall


	//top wall
	//boolean toptDoor to be made
	//if false, just a wall, if true, has a gap that is a door
	glPushMatrix();
		glColor3f(1.0, 0.0, 1.0);
		glBegin(GL_POLYGON);
			glVertex2f(-20 + wallWidth, 20);
			glVertex2f(20 - wallWidth, 20);
			glVertex2f(20 - wallWidth, 20 - wallWidth);
			glVertex2f(-20 + wallWidth, 20 - wallWidth);
		glEnd();
	glPopMatrix();

	//bottom wall
	//boolean bottomDoor to be made
	//if false, just a wall, if true, has a gap that is a door
	glPushMatrix();
		glColor3f(1.0, 0.0, 1.0);
		glBegin(GL_POLYGON);
			glVertex2f(-20 + wallWidth, -20 + wallWidth);
			glVertex2f( 20 - wallWidth, -20 + wallWidth);
			glVertex2f( 20 - wallWidth, -20);
			glVertex2f(-20 + wallWidth, -20);
		glEnd();
	glPopMatrix();

	//top left corner
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, cornerBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx - 20.0,				cCoordy + 20.0 - wallWidth);	//bottom left
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy + 20.0 - wallWidth);	//bottom right
			glTexCoord2f(1.0,  1.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy + 20.0);				//top right
			glTexCoord2f(0.0,  1.0);  glVertex2f(cCoordx - 20.0,				cCoordy + 20.0);				//top left
		glEnd();
	glPopMatrix();

	//bottom left corner
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, cornerBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0,  1.0);  glVertex2f(cCoordx - 20.0,				cCoordy - 20.0);				//bottom left
			glTexCoord2f(1.0,  1.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy - 20.0);				//bottom right
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx - 20.0 + wallWidth,	cCoordy - 20.0 + wallWidth);	//top right
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx - 20.0,				cCoordy - 20.0 + wallWidth);	//top left
		glEnd();
	glPopMatrix();

	//bottom right corner
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, cornerBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(1.0,  1.0);  glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy - 20.0);				//bottom left
			glTexCoord2f(0.0,  1.0);  glVertex2f(cCoordx + 20.0,				cCoordy - 20.0);				//bottom right
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx + 20.0,				cCoordy - 20.0 + wallWidth);	//top right
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy - 20.0 + wallWidth);	//top left
		glEnd();
	glPopMatrix();

	//top right corner
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, cornerBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(1.0,  0.0);  glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy + 20.0 - wallWidth);	//bottom left
			glTexCoord2f(0.0,  0.0);  glVertex2f(cCoordx + 20.0,				cCoordy + 20.0 - wallWidth);	//bottom right
			glTexCoord2f(0.0,  1.0);  glVertex2f(cCoordx + 20.0,				cCoordy + 20.0);				//top right
			glTexCoord2f(1.0,  1.0);  glVertex2f(cCoordx + 20.0 - wallWidth,	cCoordy + 20.0);				//top left
		glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

//1 is facing top, 2 is facing bottom, 3 is facing left, 4 is facing right
void loadSword(int direction)
{
	switch (direction)
	{
	case 1:		//facing top
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, cornerBricks);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0,  0.0);  glVertex2f(-chaSize,			0.0);
			glTexCoord2f(0.0,  1.0);  glVertex2f((chaSize * 2.0),	0.0);
			glTexCoord2f(1.0,  1.0);  glVertex2f((chaSize * 2.0),	(chaSize * 2.0));
			glTexCoord2f(1.0,  0.0);  glVertex2f(-chaSize,			(chaSize * 2.0));
		glEnd();
		glDisable(GL_TEXTURE_2D);
		break;
	case 2:		//facing bottom
		glBegin(GL_POLYGON);
			glColor3f(0.5, 0.5, 0.5); glVertex2f(-chaSize, -1);
			glColor3f(0.5, 0.5, 0.5); glVertex2f( chaSize, -1);
			glColor3f(0.5, 0.5, 0.5); glVertex2f( chaSize, -4);
			glColor3f(0.5, 0.5, 0.5); glVertex2f(-chaSize, -4);
		glEnd();
		break;
	case 3:		//facing left
		glBegin(GL_POLYGON);
			glColor3f(0.5, 0.5, 0.5); glVertex2f(-4,-chaSize-1);
			glColor3f(0.5, 0.5, 0.5); glVertex2f(-1,-chaSize-1);
			glColor3f(0.5, 0.5, 0.5); glVertex2f(-1, chaSize+1);
			glColor3f(0.5, 0.5, 0.5); glVertex2f(-4, chaSize+1);
		glEnd();
		break;
	case 4:		//facing right
		glBegin(GL_POLYGON);
			glColor3f(0.5, 0.5, 0.5); glVertex2f( 1,-chaSize-1);
			glColor3f(0.5, 0.5, 0.5); glVertex2f( 4,-chaSize-1);
			glColor3f(0.5, 0.5, 0.5); glVertex2f( 4, chaSize+1);
			glColor3f(0.5, 0.5, 0.5); glVertex2f( 1, chaSize+1);
		glEnd();
		break;
	default:
		break;
	}
}

//1 is facing top, 2 is facing left, 3 is facing bottom, 4 is facing right
void changeFacing(int direction)
{
	switch (direction)
	{
	case 1:
		cTexBegX = 0.0;
		cTexBegY = 0.8;
		tface = true;		//booleans for facing top
		lface = false;
		bface = false;
		rface = false;
		break;
	case 2:
		cTexBegX = 0.2;
		cTexBegY = 0.8;
		tface = false;		//booleans for facing bottom
		lface = false;
		bface = true;
		rface = false;
		break;
	case 3:
		cTexBegX = 0.4;
		cTexBegY = 0.8;
		tface = false;		//booleans for facing left
		lface = true;
		bface = false;
		rface = false;
		break;
	case 4:
		cTexBegX = 0.6;
		cTexBegY = 0.8;
		tface = false;		//booleans for facing right
		lface = false;
		bface = false;
		rface = true;
		break;
	default:
		tface = true;		//booleans for facing top, which is default
		lface = false;
		bface = false;
		rface = false;
		break;
	}
	cTexEndX = cTexBegX + 0.2;
	cTexEndY = cTexBegY + 0.2;
}

void bossChase()
{
	//if(Xcha Xboss Ycha Yboss) //detection for character
	float minValx = 0;
	float minValy = 0;

	if ((Xcha - Xboss) > 0)
	{
		minValx = chaseSpeed;
	}
	if ((Xcha - Xboss) < 0)
	{
		minValx = -chaseSpeed;
	}
	if ((Ycha - Yboss) > 0)
	{
		minValy = chaseSpeed;
	}
	if ((Ycha - Yboss) < 0)
	{
		minValy = -chaseSpeed;
	}

	float moveX = (minValx + ((Xcha - Xboss)*chaseSpeed)*0.8)/2;							// calculates how much to increment or decrement x by
	float moveY = (minValy + ((Ycha - Yboss)*chaseSpeed)*0.8)/2;							// calculates how much to increment or decrement y by
		
	if (Xboss < 20 - bossWidth - wallWidth && Xboss > bossWidth + wallWidth - 20)			// Stops the boss from going through left and right walls
	{
		Xboss += moveX;
	}
	else if (Xboss >= 20 - bossWidth - wallWidth)
	{
		Xboss -= 0.001;
	}
	else if (Xboss <= bossWidth + wallWidth - 20)
	{
		Xboss += 0.001;
	}

	if (Yboss < 20 - (bossHeight/2) - wallWidth && Yboss > bossHeight + wallWidth - 20)		// Stops the boss from going through walls
	{
		Yboss += moveY;
	}
	else if (Yboss >= 20 - (bossHeight/2) - wallWidth)
	{
		Yboss -= 0.001;
	}
	else if (Yboss <= bossHeight + wallWidth - 20)
	{
		Yboss += 0.001;
	}
}

void processKeys()
{
	if (keys[0x57] && (Ycha < 20 - wallWidth))	// W key
	{
		changeFacing(1);
		Ycha += chaSpeed-0.05;
	}
	if (keys[0x53] && (Ycha > wallWidth - 20 + chaSize))	// S key
	{
		changeFacing(2);
		Ycha -= chaSpeed-0.05;
	}
	if(keys[0x41] && (Xcha > wallWidth - 20 + chaSize))		// A key
	{
		changeFacing(3);
		Xcha-=chaSpeed;
	}
	if(keys[0x44] && (Xcha < 20 - chaSize - wallWidth))		// D key
	{
		changeFacing(4);
		Xcha+=chaSpeed;
	}
	if (keys[0x1B])		// ESC key
	{	
		paused = !paused;
	}
	if (keys[VK_SPACE])
	{
		attacking = true;
	} 
	if (keys[VK_SHIFT])
	{
		firing = true;
	}
	else
	{
		attacking = false;
		firing = false;
	}
}
/**************** END OPENGL FUNCTIONS *************************/

//WIN32 functions
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
void KillGLWindow();									// releases and destroys the window
bool CreateGLWindow(char* title, int width, int height); //creates the window
int WINAPI WinMain(	HINSTANCE, HINSTANCE, LPSTR, int);  // Win32 main function

//win32 global variabless
HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application


/******************* WIN32 FUNCTIONS ***************************/
int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	bool	done=false;								// Bool Variable To Exit Loop

	// Create Our OpenGL Window
	if (!CreateGLWindow("Boss Battle",screenWidth,screenHeight))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=true;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			if(keys[VK_TAB])
				done = true;

			processKeys();

			display();						// Draw The Scene
			SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (int)(msg.wParam);						// Exit The Program
}

//WIN32 Processes function - useful for responding to user inputs or other events.
LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}
		break;

		case WM_SIZE:								// Resize The OpenGL Window
		{
			reshape(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
		break;

		case WM_LBUTTONDOWN:
			{
	            mouse_x = LOWORD(lParam);          
				mouse_y = screenHeight - HIWORD(lParam);
			}
		break;

		case WM_LBUTTONUP:
			{
			}
		break;

		case WM_MOUSEMOVE:
			{
	            mouse_x = LOWORD(lParam);          
				mouse_y = screenHeight  - HIWORD(lParam);
			}
		break;
		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = true;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}
		break;
		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = false;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}
		break;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void KillGLWindow()								// Properly Kill The Window
{
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*/
 
bool CreateGLWindow(char* title, int width, int height)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;											// Return FALSE
	}
	
	dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
	dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		24,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		24,											// 24Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	reshape(width, height);					// Set Up Our Perspective GL Screen

	init();
	
	return true;									// Success
}


	/*double resultingPoint[2];
	double point[3] = {-4,-4,0};
	resultingPoint[0] = matrix[0]*point[0] + matrix[4]*point[1] + matrix[8]*point[2] + matrix[12];
	resultingPoint[1] = matrix[1]*point[0] + matrix[5]*point[1] + matrix[9]*point[2] + matrix[13];

	double matrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, matrix);*/
