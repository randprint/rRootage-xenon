/*
 * $Id: screen.h,v 1.4 2003/04/26 03:24:16 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Opengl screen functions header file.
 *
 * @version $Revision: 1.4 $
 */
#include "SDL.h"

#include <gl/gl.h>
//#include <GL/glu.h>


//Original non-gp2x code:
#define PAD_UP 1
#define PAD_DOWN 2
#define PAD_LEFT 4
#define PAD_RIGHT 8
#define PAD_BUTTON1 16
#define PAD_BUTTON2 32

//GP2X mappings
#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)

#define DEFAULT_BRIGHTNESS 224

//Convert int's to fixed or something... (Thanks Dr_Ian!)
#define Q(i,n,d) (((i)<<16)+(((n)<<16)/(d)))

#define glColor4hack(r,g,b,a) (glColor4x(r * Q(0, 1, 255), g * Q(0, 1, 255), b * Q(0, 1, 255), a * Q(0, 1, 255)))

//senquack - forcing everything to white 100% alpha did not help, in fact it corrupted the screen faster and crashes
//  ALSO changing this to nothing didn't help either (same problem)
//#define glColor4hack(r,g,b,a) glColor4x(255 * (Q(0,1,255)), 255 * (Q(0,1,255)), 255 * (Q(0,1,255)), 255 * (Q(0,1,255)))

extern float eyeX, eyeY, eyeZ;
extern float pitch, roll;
extern float zoom;
//senquack
//extern Uint8 *keys;
extern SDL_Joystick *stick;
extern int buttonReversed;
extern int lowres;
extern int windowMode;
extern int brightness;

//senquack - added to allow the laser to always be firing *except* when the fire button is pressed:
extern int laserOnByDefault;

int getPadState();
int getButtonState();

void loadModel(char *fileName, GLuint *model);
void loadGLTexture(char*, GLuint*);
void generateTexture(GLuint*);
void deleteTexture(GLuint*);
void initSDL();
void closeSDL();
void resized(int, int);
void drawGLSceneStart();
void drawGLSceneEnd();
void swapGLScene();

void gluPerspective(GLfloat fovy, GLfloat ratio, GLfloat near, GLfloat far);

void setScreenShake(int type, int cnt);
void moveScreenShake();

void drawBox(GLfloat x, GLfloat y, GLfloat width, GLfloat height, int r, int g, int b);
void drawLine(GLfloat, GLfloat, GLfloat,
	      GLfloat, GLfloat, GLfloat, int, int, int, int);
void drawLinePart(GLfloat x1, GLfloat y1, GLfloat z1,
		  GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int len);
void drawRollLineAbs(GLfloat x1, GLfloat y1, GLfloat z1,
		     GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int d1);
void drawRollLine(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
		  int r, int g, int b, int a, int d1, int d2);
void drawSquare(GLfloat x1, GLfloat y1, GLfloat z1,
		GLfloat x2, GLfloat y2, GLfloat z2,
		GLfloat x3, GLfloat y3, GLfloat z3,
		GLfloat x4, GLfloat y4, GLfloat z4,
		int r, int g, int b);
void drawStar(int f, GLfloat x, GLfloat y, GLfloat z, int r, int g, int b, float size);
void drawLaser(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
	       int cc1, int cc2, int cc3, int cc4, int cnt, int type);
void drawCore(GLfloat x, GLfloat y, int cnt, int r, int g, int b);
void drawShipShape(GLfloat x, GLfloat y, float d, int inv);
void drawBomb(GLfloat x, GLfloat y, GLfloat width, int cnt);
void drawCircle(GLfloat x, GLfloat y, GLfloat width, int cnt,
		int r1, int g1, int b1, int r2, int b2, int g2);
void drawShape(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type,
	       int r, int g, int b);
void drawShapeIka(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int c);
void drawShot(GLfloat x, GLfloat y, GLfloat d, int c, float width, float height);
void startDrawBoards();
void endDrawBoards();
void drawSideBoards();
void drawTitleBoard();

int drawNum(int n, int x ,int y, int s, int r, int g, int b);
int drawNumRight(int n, int x ,int y, int s, int r, int g, int b);
int drawNumCenter(int n, int x ,int y, int s, int r, int g, int b);
int drawTimeCenter(int n, int x ,int y, int s, int r, int g, int b);

//void CrossProd(float x1, float y1, float z1, float x2, float y2, float z2, float res[3]);

//senquack - no need for this:
//void FadiGluLookAt(float eyeX, float eyeY, float eyeZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ);

void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
	  GLfloat centerx, GLfloat centery, GLfloat centerz,
	  GLfloat upx, GLfloat upy, GLfloat upz);

//senquack - Albert had added this I think
//void drawTestPoly();

//senquack - 2/11 - disabled for now:
SDL_Surface *conv_surf_gl(SDL_Surface *s, int want_alpha);
