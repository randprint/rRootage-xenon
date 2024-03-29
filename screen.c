/*
 * $Id: screen.c,v 1.6 2003/08/10 03:21:28 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * OpenGL screen handler.
 *
 * @version $Revision: 1.6 $
 */
#include <stdio.h>
#include <stdlib.h>

#include <gl/gl.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_image.h"

#include <math.h>
#include <string.h>

#include "genmcr.h"
#include "screen.h"
#include "rr.h"
#include "degutil.h"
#include "attractmanager.h"
#include "letterrender.h"
#include "boss_mtd.h"

//#define FAR_PLANE 720
#define FAR_PLANE 720

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define LOWRES_SCREEN_WIDTH 320
#define LOWRES_SCREEN_HEIGHT 240
#define SHARE_LOC "uda://rrootage/"

static int screenWidth, screenHeight;

// Reset viewport when the screen is resized.
static void screenResized() {
  glViewport(0, 0, screenWidth, screenHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
	gluPerspective(45.0f, (GLfloat)screenWidth/(GLfloat)screenHeight, 0.1f, FAR_PLANE);
	glMatrixMode(GL_MODELVIEW);
}

void resized(int width, int height) {
  screenWidth = width; screenHeight = height;
  screenResized();
}

//Added for gpu940
void gluPerspective(GLfloat fovy, GLfloat ratio, GLfloat near, GLfloat far)
{
	GLfloat top = near * tan(fovy * M_PI/360.0f);
	GLfloat bottom = -top;
	GLfloat right = top * ratio;
	GLfloat left = -right;

	glFrustum(left, right, bottom, top, near, far);
} 

// Init OpenGL.
static void initGL() 
{
  printf("Opening Xenos OpenGL\n"); 

  XenonGLInit();

  printf("Opened Xenos OpenGL\n");
  glViewport(0, 0, screenWidth, screenHeight);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glLineWidth(1);
  glEnable(GL_LINE_SMOOTH);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  

  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_COLOR_MATERIAL);

  resized(screenWidth, screenHeight);
}


// Load bitmaps and convert to textures.
void loadGLTexture(char *fileName, GLuint *texture) 
{
  SDL_Surface *surface;
  int mode=0; //The bit-depth of the texture.
  char name[32];

  strcpy(name, SHARE_LOC);
  strcat(name, "images/");
  strcat(name, fileName);

  //surface = SDL_LoadBMP(name);
  surface = IMG_Load(name); //Changed by Albert... this will load any image (hopefully transparent PNGs)
  if ( !surface ) {
    fprintf(stderr, "Unable to load texture: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }
  
  surface = conv_surf_gl(surface, surface->format->Amask || (surface->flags & SDL_SRCCOLORKEY));


	//Attempted hackery to make transparencies/color-keying work - Albert
	SDL_PixelFormat RGBAFormat;
	RGBAFormat.palette = 0; RGBAFormat.colorkey = 0; RGBAFormat.alpha = 0;
	RGBAFormat.BitsPerPixel = 32; RGBAFormat.BytesPerPixel = 4;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	RGBAFormat.Rmask = 0xFF000000; RGBAFormat.Rshift = 0; RGBAFormat.Rloss = 0;
	RGBAFormat.Gmask = 0x00FF0000; RGBAFormat.Gshift = 8; RGBAFormat.Gloss = 0;
	RGBAFormat.Bmask = 0x0000FF00; RGBAFormat.Bshift = 16; RGBAFormat.Bloss = 0;
	RGBAFormat.Amask = 0x000000FF; RGBAFormat.Ashift = 24; RGBAFormat.Aloss = 0;
	#else
	RGBAFormat.Rmask = 0x000000FF; RGBAFormat.Rshift = 24; RGBAFormat.Rloss = 0;
	RGBAFormat.Gmask = 0x0000FF00; RGBAFormat.Gshift = 16; RGBAFormat.Gloss = 0;
	RGBAFormat.Bmask = 0x00FF0000; RGBAFormat.Bshift = 8; RGBAFormat.Bloss = 0;
	RGBAFormat.Amask = 0xFF000000; RGBAFormat.Ashift = 0; RGBAFormat.Aloss = 0;
	#endif



  // Create the target alpha surface with correct color component ordering 

  SDL_Surface *alphaImage = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w,
 	surface->h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN // OpenGL RGBA masks 
   0x000000FF, 
   0x0000FF00, 
   0x00FF0000, 
   0xFF000000
#else
   0xFF000000,
   0x00FF0000, 
   0x0000FF00, 
   0x000000FF
#endif
  );
  
  if (alphaImage == 0)
  	printf("ruh oh, alphaImage creation failed in loadGLTexture() (screen.c)\n");


  // Set up so that colorkey pixels become transparent :
  Uint32 colorkey = SDL_MapRGBA(alphaImage->format, 255, 255, 0, 0); //R=255, G=255, B=0
  SDL_FillRect(alphaImage, 0, colorkey);

  colorkey = SDL_MapRGBA(surface->format, 255, 255, 0, 0 );
  SDL_SetColorKey(surface, SDL_SRCCOLORKEY, colorkey);


  SDL_Rect area;
 
  // Copy the surface into the GL texture image : 
  area.x = 0;
  area.y = 0; 
  area.w = surface->w;
  area.h = surface->h;
  SDL_BlitSurface(surface, &area, alphaImage, &area);

 // for (int i = 0; i < conv->w * conv->h; i++)
 // {
 //     
 // }

  SDL_Surface *conv = SDL_ConvertSurface(surface, &RGBAFormat, SDL_SWSURFACE);

//http://osdl.sourceforge.net/OSDL/OSDL-0.3/src/doc/web/main/documentation/rendering/SDL-openGL-examples.html
//http://osdl.sourceforge.net/main/documentation/rendering/SDL-openGL.html --> Good explainations

    // work out what format to tell glTexImage2D to use...
    if (surface->format->BytesPerPixel == 3) { // RGB 24bit
		mode = GL_RGB;
    } else if (surface->format->BytesPerPixel == 4) { // RGBA 32bit
        mode = GL_RGBA;
    }
    else {
    	printf("loadGLTexture: Error: Weird RGB mode found in surface.\n");
    	SDL_Quit();
    }

	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, surface->w, surface->h, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
	
  if (surface == NULL)
  {
     printf("Error: surface NULL in loadGLTexture.\n");
     SDL_Quit();
  }
  
	//Added by Albert (somehow Kenta Cho managed to get OpenGL to render SDL textures, looks like)
	glTexImage2D( GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0,
				  mode, GL_UNSIGNED_BYTE, surface->pixels );
				  
	//Added by Albert
	if (surface)
		SDL_FreeSurface(surface);     
	if (alphaImage)
		SDL_FreeSurface(alphaImage);   
	if (conv)
		SDL_FreeSurface(conv);

}

void generateTexture(GLuint *texture) {
  glGenTextures(1, texture);
}

void deleteTexture(GLuint *texture) {
  glDeleteTextures(1, texture);
}

static GLuint starTexture;
#define STAR_BMP "star.bmp"
static GLuint smokeTexture;
#define SMOKE_BMP "smoke.bmp"
static GLuint titleTexture;
#define TITLE_BMP "title.bmp"

int lowres = 0;
int windowMode = 0;
int brightness = DEFAULT_BRIGHTNESS;
Uint8 *keys;
SDL_Joystick *stick = NULL;
int joystickMode = 1;


void initSDL() {
  Uint32 videoFlags;

  if ( lowres ) {
    screenWidth  = LOWRES_SCREEN_WIDTH;
    screenHeight = LOWRES_SCREEN_HEIGHT;
  } else {
    screenWidth  = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;
  }

  /* Initialize SDL */
 if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }
  
  if ( SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0 ) {
  //if ( SDL_Init(SDL_INIT_JOYSTICK) < 0 ) {
    fprintf(stderr, "Unable to initialize SDL_JOYSTICK: %s\n", SDL_GetError());
    joystickMode = 0;
    exit(1);
  }
  
//Changed for gpu940:
  /* Create an OpenGL screen */
  //if ( windowMode ) {
  	
  	//videoFlags = SDL
    //videoFlags = SDL_OPENGL | SDL_RESIZABLE;
  //} else {
   // videoFlags = SDL_OPENGL | SDL_FULLSCREEN;
  //}
//videoFlags = SDL_SWSURFACE;

if ( SDL_SetVideoMode(screenWidth, screenHeight, 16, videoFlags) == NULL ) {
    fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
    SDL_Quit();
    exit(2);
  }

  if (joystickMode == 1) {
  	SDL_JoystickEventState(SDL_ENABLE);
    stick = SDL_JoystickOpen(0);
  }

  /* Set the title bar in environments that support it */
  SDL_WM_SetCaption(CAPTION, NULL);

  initGL();
  loadGLTexture(STAR_BMP, &starTexture);
  loadGLTexture(SMOKE_BMP, &smokeTexture);
  loadGLTexture(TITLE_BMP, &titleTexture);

  SDL_ShowCursor(SDL_DISABLE);
  
}

void closeSDL() {
	//senquack
//  SDL_ShowCursor(SDL_ENABLE);
}

float zoom = 20;//15;
static int screenShakeCnt = 0;
static int screenShakeType = 0;


static void setEyepos() {
  float x, y;
  glPushMatrix();
  if ( screenShakeCnt > 0 ) {
    switch ( screenShakeType ) {
    case 0:
      x = (float)randNS2(256)/5000.0f;
      y = (float)randNS2(256)/5000.0f;
      break;
    default:
      x = (float)randNS2(256)*screenShakeCnt/21000.0f;
      y = (float)randNS2(256)*screenShakeCnt/21000.0f;
      break;
    }
    gluLookAt(0, 0, zoom, x, y, 0, 0.0f, 1.0f, 0.0f);
  } else {
		 gluLookAt(0, 0, zoom, 0, 0, 0, 0.0f, 1.0f, 0.0f);
  }
}

void setScreenShake(int type, int cnt) {
  screenShakeType = type; screenShakeCnt = cnt;
}

void moveScreenShake() {
  if ( screenShakeCnt > 0 ) {
    screenShakeCnt--;
  }
}


void drawGLSceneStart() {
	XenonBeginGl();
  glClear(GL_COLOR_BUFFER_BIT);
  setEyepos();
}

void drawGLSceneEnd() {
  XenonEndGl();
  glPopMatrix();

}

void swapGLScene() {
	SDL_GL_SwapBuffers();
	//Switched to XenonGLDisplay() for xenos
	//XenonGLDisplay();
  //XenonEndGl();
}


void drawBox(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
	     int r, int g, int b) {
  glPushMatrix();
  glTranslatef(x, y, 0);
  glColor4ub(r, g, b, 128);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(-width, -height,  0);
  glVertex3f( width, -height,  0);
  glVertex3f( width,  height,  0);
  glVertex3f(-width,  height,  0);
  glEnd();
  glColor4ub(r, g, b, 255);
  glBegin(GL_LINE_STRIP);
  glVertex3f(-width, -height,  0);
  glVertex3f( width, -height,  0);
  glVertex3f( width,  height,  0);
  glVertex3f(-width,  height,  0);
  glVertex3f(-width, -height,  0);
  glEnd();
  glPopMatrix();
}

void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
	      GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
  glColor4ub(r, g, b, a);
  glBegin(GL_LINES);
  glVertex3f(x1, y1, z1);
  glVertex3f(x2, y2, z2);
  glEnd();
}

void drawLinePart(GLfloat x1, GLfloat y1, GLfloat z1,
		  GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int len) {
  glColor4ub(r, g, b, a);
  glBegin(GL_LINES);
  glVertex3f(x1, y1, z1);
  glVertex3f(x1+(x2-x1)*len/256, y1+(y2-y1)*len/256, z1+(z2-z1)*len/256);
  glEnd();
}

void drawRollLineAbs(GLfloat x1, GLfloat y1, GLfloat z1,
		     GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int d1) {
  glPushMatrix();
  glRotatef((float)d1*360/1024, 0, 0, 1);
  glColor4ub(r, g, b, a);
  glBegin(GL_LINES);
  glVertex3f(x1, y1, z1);
  glVertex3f(x2, y2, z2);
  glEnd();
  glPopMatrix();
}


void drawRollLine(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
		  int r, int g, int b, int a, int d1, int d2) {
  glPushMatrix();
  glTranslatef(x, y, z);
  glRotatef((float)d1*360/1024, 0, 0, 1);
  glRotatef((float)d2*360/1024, 1, 0, 0);
  glColor4ub(r, g, b, a);
  glBegin(GL_LINES);
  glVertex3f(0, -width, 0);
  glVertex3f(0,  width, 0);
  glEnd();
  glPopMatrix();
}

void drawSquare(GLfloat x1, GLfloat y1, GLfloat z1,
		GLfloat x2, GLfloat y2, GLfloat z2,
		GLfloat x3, GLfloat y3, GLfloat z3,
		GLfloat x4, GLfloat y4, GLfloat z4,
		int r, int g, int b) {
  glColor4ub(r, g, b, 64);

  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(x1, y1, z1);
  glVertex3f(x2, y2, z2);
  glVertex3f(x3, y3, z3);
  glVertex3f(x4, y4, z4);
  glEnd();
}

void drawStar(int f, GLfloat x, GLfloat y, GLfloat z, int r, int g, int b, float size) {
  glEnable(GL_TEXTURE_2D);
  if ( f ) {
    glBindTexture(GL_TEXTURE_2D, starTexture);
  } else {
    glBindTexture(GL_TEXTURE_2D, smokeTexture);
  }
  glColor4ub(r, g, b, 255);
  glPushMatrix();
  glTranslatef(x, y, z);
  glRotatef(rand()%360, 0.0f, 0.0f, 1.0f);
  glBegin(GL_TRIANGLE_FAN);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(-size, -size,  0);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f( size, -size,  0);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f( size,  size,  0);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-size,  size,  0);
  glEnd();
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);
}

#define LASER_ALPHA 100
#define LASER_LINE_ALPHA 50
#define LASER_LINE_ROLL_SPEED 17
#define LASER_LINE_UP_SPEED 16


void drawLaser(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
	       int cc1, int cc2, int cc3, int cc4, int cnt, int type) {
  int i, d;
  float gx, gy;


  glBegin(GL_TRIANGLE_FAN);
  if ( type != 0 ) {
    glColor4ub(cc1, cc1, cc1, LASER_ALPHA);
    glVertex3f(x-width, y, 0);
  }
  glColor4ub(cc2, 255, cc2, LASER_ALPHA);
  glVertex3f(x, y, 0);
  glColor4ub(cc4, 255, cc4, LASER_ALPHA);
  glVertex3f(x, y+height, 0);
  glColor4ub(cc3, cc3, cc3, LASER_ALPHA);
  glVertex3f(x-width, y+height, 0);
  glEnd();
  glBegin(GL_TRIANGLE_FAN);
  if ( type != 0 ) {
    glColor4ub(cc1, cc1, cc1, LASER_ALPHA);
    glVertex3f(x+width, y, 0);
  }
  glColor4ub(cc2, 255, cc2, LASER_ALPHA);
  glVertex3f(x, y, 0);
  glColor4ub(cc4, 255, cc4, LASER_ALPHA);
  glVertex3f(x, y+height, 0);
  glColor4ub(cc3, cc3, cc3, LASER_ALPHA);
  glVertex3f(x+width, y+height, 0);
  glEnd();
  if ( type == 2 ) return;
  glColor4ub(80, 240, 80, LASER_LINE_ALPHA);
  glBegin(GL_LINES);
  d = (cnt*LASER_LINE_ROLL_SPEED)&(512/4-1);
  for ( i=0 ; i<4 ; i++, d+=(512/4) ) {
    d &= 1023;
    gx = x + width*sctbl[d+256]/256.0f;
    if ( type == 1 ) {
      glVertex3f(gx, y, 0);
    } else {
      glVertex3f(x, y, 0);
    }
    glVertex3f(gx, y+height, 0);
  }
  if ( type == 0 ) {
    glEnd();
    return;
  }
  gy = y + (height/4/LASER_LINE_UP_SPEED) * (cnt&(LASER_LINE_UP_SPEED-1));
  for ( i=0 ; i<4 ; i++, gy+=height/4 ) {
    glVertex3f(x-width, gy, 0);
    glVertex3f(x+width, gy, 0);
  }
  glEnd();

}

#define SHAPE_POINT_SIZE 0.05f
#define SHAPE_BASE_COLOR_R 250
#define SHAPE_BASE_COLOR_G 240
#define SHAPE_BASE_COLOR_B 180

#define CORE_HEIGHT 0.2f
#define CORE_RING_SIZE 0.6f

#define SHAPE_POINT_SIZE_L 0.07f

static void drawRing(GLfloat x, GLfloat y, int d1, int d2, int r, int g, int b) {
  int i, d;
  float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
  glPushMatrix();
  glTranslatef(x, y, 0);
  glRotatef((float)d1*360/1024, 0, 0, 1);
  glRotatef((float)d2*360/1024, 1, 0, 0);
  glColor4ub(r, g, b, 255);
  x1 = x2 = 0;
  y1 = y4 =  CORE_HEIGHT/2;
  y2 = y3 = -CORE_HEIGHT/2;
  z1 = z2 = CORE_RING_SIZE;
  for ( i=0,d=0 ; i<8 ; i++ ) {
    d+=(1024/8); d &= 1023;
    x3 = x4 = sctbl[d+256]*CORE_RING_SIZE/256;
    z3 = z4 = sctbl[d]    *CORE_RING_SIZE/256;
    drawSquare(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
    x1 = x3; y1 = y3; z1 = z3;
    x2 = x4; y2 = y4; z2 = z4;
  }
  glPopMatrix();
}


void drawCore(GLfloat x, GLfloat y, int cnt, int r, int g, int b) {
  int i;
  float cy;
  glPushMatrix();
  glTranslatef(x, y, 0);
  glColor4ub(r, g, b, 255);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
  glVertex3f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
  glVertex3f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
  glEnd();

  glPopMatrix();
  cy = y - CORE_HEIGHT*2.5f;
  for ( i=0 ; i<4 ; i++, cy+=CORE_HEIGHT ) {
    drawRing(x, cy, (cnt*(4+i))&1023, (sctbl[(cnt*(5+i))&1023]/4)&1023, r, g, b);
  }
}

#define SHIP_DRUM_R 0.4f
#define SHIP_DRUM_WIDTH 0.05f
#define SHIP_DRUM_HEIGHT 0.35f

void drawShipShape(GLfloat x, GLfloat y, float d, int inv) {
  int i;
  glPushMatrix();
  glTranslatef(x, y, 0);
  glColor4ub(255, 100, 100, 255);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
  glVertex3f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
  glVertex3f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
  glEnd();

  if ( inv ) {
    glPopMatrix();
    return;
  }
  glRotatef(d, 0, 1, 0);
    glColor4ub(120, 220, 100, 150);
    /*if ( mode == IKA_MODE ) {
    glColor4ub(180, 200, 160, 150);
  } else {
    glColor4ub(120, 220, 100, 150);
    }*/
  for ( i=0 ; i<8 ; i++ ) {
    glRotatef(45, 0, 1, 0);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
    glVertex3f( SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
    glVertex3f( SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
    glVertex3f(-SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
    glVertex3f(-SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
    glEnd();
  }
  glPopMatrix();
}

void drawBomb(GLfloat x, GLfloat y, GLfloat width, int cnt) {
  int i, d, od, c;
  GLfloat x1, y1, x2, y2;
  d = cnt*48; d &= 1023;
  c = 4+(cnt>>3); if ( c > 16 ) c = 16;
  od = 1024/c;
  x1 = (sctbl[d]    *width)/256 + x;
  y1 = (sctbl[d+256]*width)/256 + y;


  for ( i=0 ; i<c ; i++ ) {
    d += od; d &= 1023;
    x2 = (sctbl[d]    *width)/256 + x;
    y2 = (sctbl[d+256]*width)/256 + y;
    drawLine(x1, y1, 0, x2, y2, 0, 255, 255, 255, 255);
    x1 = x2; y1 = y2;
  }
  glEnd();
}


void drawCircle(GLfloat x, GLfloat y, GLfloat width, int cnt,
		int r1, int g1, int b1, int r2, int b2, int g2) {
  int i, d;
  GLfloat x1, y1, x2, y2;
  if ( (cnt&1) == 0 ) {
    glColor4ub(r1, g1, b1, 64);
  } else {
    glColor4ub(255, 255, 255, 64);
  }
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(x, y, 0);
  d = cnt*48; d &= 1023;
  x1 = (sctbl[d]    *width)/256 + x;
  y1 = (sctbl[d+256]*width)/256 + y;
  glColor4ub(r2, g2, b2, 150);
  for ( i=0 ; i<16 ; i++ ) {
    d += 64; d &= 1023;
    x2 = (sctbl[d]    *width)/256 + x;
    y2 = (sctbl[d+256]*width)/256 + y;
    glVertex3f(x1, y1, 0);
    glVertex3f(x2, y2, 0);
    x1 = x2; y1 = y2;
  }
  glEnd();
}


void drawShape(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type,
	       int r, int g, int b) {
  GLfloat sz, sz2;
  glPushMatrix();
  glTranslatef(x, y, 0);
  glColor4ub(r, g, b, 255);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
  glEnd();

  switch ( type ) {
  case 0:
    sz = size/2;
    glRotatef((float)d*360/1024, 0, 0, 1);

    glDisable(GL_BLEND);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-sz, -sz,  0);
    glVertex3f( sz, -sz,  0);
    glVertex3f( 0, size,  0);
     glVertex3f(-sz, -sz,  0);
   glEnd();
    glEnable(GL_BLEND);
    glColor4ub(r, g, b, 150);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-sz, -sz,  0);
    glVertex3f( sz, -sz,  0);
    glColor4ub(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
    glVertex3f( 0, size,  0);
    glEnd();
    break;
  case 1:
    sz = size/2;
    glRotatef((float)((cnt*23)&1023)*360/1024, 0, 0, 1);

    glDisable(GL_BLEND);
    glBegin(GL_LINE_STRIP);
    glVertex3f(  0, -size,  0);
    glVertex3f( sz,     0,  0);
    glVertex3f(  0,  size,  0);
    glVertex3f(-sz,     0,  0);
    glVertex3f(  0, -size,  0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4ub(r, g, b, 180);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(  0, -size,  0);
    glVertex3f( sz,     0,  0);
    glColor4ub(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
    glVertex3f(  0,  size,  0);
    glVertex3f(-sz,     0,  0);
    glEnd();
    break;
  case 2:
    sz = size/4; sz2 = size/3*2;
    glRotatef((float)d*360/1024, 0, 0, 1);
    glDisable(GL_BLEND);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-sz, -sz2,  0);
    glVertex3f( sz, -sz2,  0);
    glVertex3f( sz,  sz2,  0);
    glVertex3f(-sz,  sz2,  0);
    glVertex3f(-sz, -sz2,  0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4ub(r, g, b, 120);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-sz, -sz2,  0);
    glVertex3f( sz, -sz2,  0);
    glColor4ub(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
    glVertex3f( sz, sz2,  0);
    glVertex3f(-sz, sz2,  0);
    glEnd();
    break;
  case 3:
    sz = size/2;
    glRotatef((float)((cnt*37)&1023)*360/1024, 0, 0, 1);
    glDisable(GL_BLEND);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-sz, -sz,  0);
    glVertex3f( sz, -sz,  0);
    glVertex3f( sz,  sz,  0);
    glVertex3f(-sz,  sz,  0);
    glVertex3f(-sz, -sz,  0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4ub(r, g, b, 180);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-sz, -sz,  0);
    glVertex3f( sz, -sz,  0);
    glColor4ub(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
    glVertex3f( sz,  sz,  0);
    glVertex3f(-sz,  sz,  0);
    glEnd();
    break;
  case 4:
    sz = size/2;
    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
    glDisable(GL_BLEND);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-sz/2, -sz,  0);
    glVertex3f( sz/2, -sz,  0);
    glVertex3f( sz,  -sz/2,  0);
    glVertex3f( sz,   sz/2,  0);
    glVertex3f( sz/2,  sz,  0);
    glVertex3f(-sz/2,  sz,  0);
    glVertex3f(-sz,   sz/2,  0);
    glVertex3f(-sz,  -sz/2,  0);
    glVertex3f(-sz/2, -sz,  0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4ub(r, g, b, 220);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-sz/2, -sz,  0);
    glVertex3f( sz/2, -sz,  0);
    glVertex3f( sz,  -sz/2,  0);
    glVertex3f( sz,   sz/2,  0);
    glColor4ub(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
    glVertex3f( sz/2,  sz,  0);
    glVertex3f(-sz/2,  sz,  0);
    glVertex3f(-sz,   sz/2,  0);
    glVertex3f(-sz,  -sz/2,  0);
    glEnd();
    break;
  case 5:
    sz = size*2/3; sz2 = size/5;
    glRotatef((float)d*360/1024, 0, 0, 1);
    glDisable(GL_BLEND);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-sz, -sz+sz2,  0);
    glVertex3f( 0, sz+sz2,  0);
    glVertex3f( sz, -sz+sz2,  0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4ub(r, g, b, 150);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-sz, -sz+sz2,  0);
    glVertex3f( sz, -sz+sz2,  0);
    glColor4ub(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
    glVertex3f( 0, sz+sz2,  0);
    glEnd();
    break;
  case 6:
    sz = size/2;
    glRotatef((float)((cnt*13)&1023)*360/1024, 0, 0, 1);
    glDisable(GL_BLEND);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-sz, -sz,  0);
    glVertex3f(  0, -sz,  0);
    glVertex3f( sz,   0,  0);
    glVertex3f( sz,  sz,  0);
    glVertex3f(  0,  sz,  0);
    glVertex3f(-sz,   0,  0);
    glVertex3f(-sz, -sz,  0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4ub(r, g, b, 210);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-sz, -sz,  0);
    glVertex3f(  0, -sz,  0);
    glVertex3f( sz,   0,  0);
    glColor4ub(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
    glVertex3f( sz,  sz,  0);
    glVertex3f(  0,  sz,  0);
    glVertex3f(-sz,   0,  0);
    glEnd();
    break;
  }
  glPopMatrix();
}
static int ikaClr[2][3][3] = {
  {{230, 230, 255}, {100, 100, 200}, {50, 50, 150}},
  {{0, 0, 0}, {200, 0, 0}, {100, 0, 0}},
};


void drawShapeIka(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int c) {
  GLfloat sz, sz2, sz3;
  glPushMatrix();
  glTranslatef(x, y, 0);
  glColor4ub(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
  glDisable(GL_BLEND);

   glBegin(GL_TRIANGLE_FAN);
  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
  glEnd();
  glColor4ub(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
  switch ( type ) {
  case 0:
    sz = size/2; sz2 = sz/3; sz3 = size*2/3;
    glRotatef((float)d*360/1024, 0, 0, 1);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-sz, -sz3,  0);
    glVertex3f( sz, -sz3,  0);
    glVertex3f( sz2, sz3,  0);
    glVertex3f(-sz2, sz3,  0);
    glVertex3f(-sz, -sz3,  0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4ub(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-sz, -sz3,  0);
    glVertex3f( sz, -sz3,  0);
    glColor4ub(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
    glVertex3f( sz2, sz3,  0);
    glVertex3f(-sz2, sz3,  0);
    glEnd();
    break;
  case 1:
    sz = size/2;
    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-sz/2, -sz,  0);
    glVertex3f( sz/2, -sz,  0);
    glVertex3f( sz,  -sz/2,  0);
    glVertex3f( sz,   sz/2,  0);
    glVertex3f( sz/2,  sz,  0);
    glVertex3f(-sz/2,  sz,  0);
    glVertex3f(-sz,   sz/2,  0);
    glVertex3f(-sz,  -sz/2,  0);
    glVertex3f(-sz/2, -sz,  0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4ub(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-sz/2, -sz,  0);
    glVertex3f( sz/2, -sz,  0);
    glVertex3f( sz,  -sz/2,  0);
    glVertex3f( sz,   sz/2,  0);
    glColor4ub(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
    glVertex3f( sz/2,  sz,  0);
    glVertex3f(-sz/2,  sz,  0);
    glVertex3f(-sz,   sz/2,  0);
    glVertex3f(-sz,  -sz/2,  0);
    glEnd();
    break;
  }
  glPopMatrix();
}

#define SHOT_WIDTH 0.1
#define SHOT_HEIGHT 0.2

static int shtClr[3][3][3] = {
  {{200, 200, 225}, {50, 50, 200}, {200, 200, 225}},
  {{100, 0, 0}, {100, 0, 0}, {200, 0, 0}},
  {{100, 200, 100}, {50, 100, 50}, {100, 200, 100}},
};


void drawShot(GLfloat x, GLfloat y, GLfloat d, int c, float width, float height) {
  glPushMatrix();
  glTranslatef(x, y, 0);
  glRotatef(d, 0, 0, 1);

  glColor4ub(shtClr[c][0][0], shtClr[c][0][1], shtClr[c][0][2], 240);
  glDisable(GL_BLEND);
  glBegin(GL_LINES);
  glVertex3f(-width, -height, 0);
  glVertex3f(-width,  height, 0);
  glVertex3f( width, -height, 0);
  glVertex3f( width,  height, 0);
  glEnd();
  glEnable(GL_BLEND);

  glColor4ub(shtClr[c][1][0], shtClr[c][1][1], shtClr[c][1][2], 240);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(-width, -height, 0);
  glVertex3f( width, -height, 0);
  glColor4ub(shtClr[c][2][0], shtClr[c][2][1], shtClr[c][2][2], 240);
  glVertex3f( width,  height, 0);
  glVertex3f(-width,  height, 0);
  glEnd();
  glPopMatrix();
}


void startDrawBoards() {
  glMatrixMode(GL_PROJECTION);
  //senquack - dunno why we do this
  //glPushMatrix();
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void endDrawBoards() {
	glPopMatrix();
	//really?
	screenResized();
}

static void drawBoard(int x, int y, int width, int height) {
  glColor4ub(0, 0, 0, 255);
  glBegin(GL_QUADS);
  glVertex2f(x,y);
  glVertex2f(x+width,y);
  glVertex2f(x+width,y+height);
  glVertex2f(x,y+height);
  glEnd();
}

void drawSideBoards() {
  glDisable(GL_BLEND);
  drawBoard(0, 0, 160, 480);
  drawBoard(480, 0, 160, 480);
  glEnable(GL_BLEND);
  drawScore();
  drawRPanel();
}


void drawTitleBoard() {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, titleTexture);
  glColor4ub(255, 255, 255, 255);
  glBegin(GL_TRIANGLE_FAN);
  glTexCoord2f(0.0f, 0.0f); 
  glVertex3f(350, 78,  0);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(470, 78,  0);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(470, 114,  0);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(350, 114,  0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glColor4ub(200, 200, 200, 255);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(350, 30, 0);
  glVertex3f(400, 30, 0);
  glVertex3f(380, 56, 0);
  glVertex3f(380, 80, 0);
  glVertex3f(350, 80, 0);
  glEnd();
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(404, 80, 0);
  glVertex3f(404, 8, 0);
  glVertex3f(440, 8, 0);
  glVertex3f(440, 44, 0);
  glVertex3f(465, 80, 0);
  glEnd();

  glColor4ub(255, 255, 255, 255);
  glBegin(GL_LINE_STRIP);
  glVertex3f(350, 30, 0);
  glVertex3f(400, 30, 0);
  glVertex3f(380, 56, 0);
  glVertex3f(380, 80, 0);
  glVertex3f(350, 80, 0);
  glVertex3f(350, 30, 0);
  glVertex3f(350, 30, 0);
  glEnd();

  glBegin(GL_LINE_STRIP);
  glVertex3f(404, 80, 0);
  glVertex3f(404, 8, 0);
  glVertex3f(440, 8, 0);
  glVertex3f(440, 44, 0);
  glVertex3f(465, 80, 0);
  glVertex3f(404, 80, 0);
  glEnd();
}

// Draw the numbers.
int drawNum(int n, int x ,int y, int s, int r, int g, int b) {
  for ( ; ; ) {
    drawLetter(n%10, x, y, s, 3, r, g, b);
    y += s*1.7f;
    n /= 10;
    if ( n <= 0 ) break;
  }
  return y;
}

int drawNumRight(int n, int x ,int y, int s, int r, int g, int b) {
  int d, nd, drawn = 0;
  for ( d = 100000000 ; d > 0 ; d /= 10 ) {
    nd = (int)(n/d);
    if ( nd > 0 || drawn ) {
      n -= d*nd;
      drawLetter(nd%10, x, y, s, 1, r, g, b);
      y += s*1.7f;
      drawn = 1;
    }
  }
  if ( !drawn ) {
    drawLetter(0, x, y, s, 1, r, g, b);
    y += s*1.7f;
  }
  return y;
}

int drawNumCenter(int n, int x ,int y, int s, int r, int g, int b) {
  for ( ; ; ) {
    drawLetter(n%10, x, y, s, 0, r, g, b);
    x -= s*1.7f;
    n /= 10;
    if ( n <= 0 ) break;
  }
  return y;
}

int drawTimeCenter(int n, int x ,int y, int s, int r, int g, int b) {
  int i;
  for ( i=0 ; i<7 ; i++ ) {
    if ( i != 4 ) {
      drawLetter(n%10, x, y, s, 0, r, g, b);
      n /= 10;
    } else {
      drawLetter(n%6, x, y, s, 0, r, g, b);
      n /= 6;
    }
    if ( (i&1) == 1 || i == 0 ) {
      switch ( i ) {
      case 3:
	drawLetter(41, x+s*1.16f, y, s, 0, r, g, b);
	break;
      case 5:
	drawLetter(40, x+s*1.16f, y, s, 0, r, g, b);
	break;
      }
      x -= s*1.7f;
    } else {
      x -= s*2.2f;
    }
    if ( n <= 0 ) break;
  }
  return y;
}

#define JOYSTICK_AXIS 8000

int getPadState() {
  int x = 0, y = 0;
  int pad = 0;
  int gp2x_up = 0, gp2x_upleft = 0, gp2x_left = 0, gp2x_downleft = 0, gp2x_down = 0, 
  	  gp2x_downright = 0, gp2x_right = 0, gp2x_upright = 0; 
  if ( stick != NULL ) {
    x = SDL_JoystickGetAxis(stick, 0);
    y = SDL_JoystickGetAxis(stick, 1);

    gp2x_up = SDL_JoystickGetButton(stick, GP2X_BUTTON_UP);
    gp2x_upleft = SDL_JoystickGetButton(stick, GP2X_BUTTON_UPLEFT);
    gp2x_left = SDL_JoystickGetButton(stick, GP2X_BUTTON_LEFT);
    gp2x_downleft = SDL_JoystickGetButton(stick, GP2X_BUTTON_DOWNLEFT);
    gp2x_down = SDL_JoystickGetButton(stick, GP2X_BUTTON_DOWN);
    gp2x_downright = SDL_JoystickGetButton(stick, GP2X_BUTTON_DOWNRIGHT);
    gp2x_right = SDL_JoystickGetButton(stick, GP2X_BUTTON_RIGHT);
    gp2x_upright = SDL_JoystickGetButton(stick, GP2X_BUTTON_UPRIGHT);  

  }
  if ( gp2x_right || gp2x_downright || gp2x_upright || x > JOYSTICK_AXIS ) {
    pad |= PAD_RIGHT;
  }
  if ( gp2x_left || gp2x_downleft || gp2x_upleft || x < -JOYSTICK_AXIS ) {
    pad |= PAD_LEFT;
  }
  if ( gp2x_down || gp2x_downright || gp2x_downleft || y > JOYSTICK_AXIS ) {
    pad |= PAD_DOWN;
  }
  if ( gp2x_up || gp2x_upright || gp2x_upleft || y < -JOYSTICK_AXIS ) {
    pad |= PAD_UP;
  }
    return pad;
}

int buttonReversed = 0;

//senquack - added to allow the laser to always be firing *except* when the fire button is pressed:
int laserOnByDefault = 0;

int getButtonState() {
  int btn = 0;
  int btn1 = 0, btn2 = 0, btn3 = 0, btn4 = 0, btn5 = 0, btn6 = 0;
  if ( stick != NULL ) {
    btn1 = SDL_JoystickGetButton(stick, GP2X_BUTTON_B) |
				 SDL_JoystickGetButton(stick, GP2X_BUTTON_R);
    btn2 = SDL_JoystickGetButton(stick, GP2X_BUTTON_X) |
				SDL_JoystickGetButton(stick, GP2X_BUTTON_L);
    btn3 = SDL_JoystickGetButton(stick, GP2X_BUTTON_Y);
    btn4 = SDL_JoystickGetButton(stick, GP2X_BUTTON_A);
    btn5 = SDL_JoystickGetButton(stick, GP2X_BUTTON_START);
    btn6 = SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT);
  }
  if ( btn1 || btn3 ) {
    if ( !buttonReversed ) {
      btn |= PAD_BUTTON1;
    } else {
      btn |= PAD_BUTTON2;
    }
  }
  if ( btn2 || btn4 ) {
    if ( !buttonReversed ) {
      btn |= PAD_BUTTON2;
    } else {
      btn |= PAD_BUTTON1;
    }
  }
  return btn;
}


//senquack - it appears that Albert added these implementations of gluLookAt:
//Mesa's implementation, switched to floats from doubles in a desparate attempt at getting more speed.
void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
	  GLfloat centerx, GLfloat centery, GLfloat centerz,
	  GLfloat upx, GLfloat upy, GLfloat upz)
{
   GLfloat m[16];
   GLfloat x[3], y[3], z[3];
   GLfloat mag;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = eyex - centerx;
   z[1] = eyey - centery;
   z[2] = eyez - centerz;
   mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
   if (mag) {			/* mpichler, 19950515 */
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
   }

   /* Y vector */
   y[0] = upx;
   y[1] = upy;
   y[2] = upz;

   /* X vector = Y cross Z */
   x[0] = y[1] * z[2] - y[2] * z[1];
   x[1] = -y[0] * z[2] + y[2] * z[0];
   x[2] = y[0] * z[1] - y[1] * z[0];

   /* Recompute Y = Z cross X */
   y[0] = z[1] * x[2] - z[2] * x[1];
   y[1] = -z[0] * x[2] + z[2] * x[0];
   y[2] = z[0] * x[1] - z[1] * x[0];

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

   mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
   if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
   }

#define M(row,col)  m[col*4+row]
   M(0, 0) = x[0];
   M(0, 1) = x[1];
   M(0, 2) = x[2];
   M(0, 3) = 0.0;
   M(1, 0) = y[0];
   M(1, 1) = y[1];
   M(1, 2) = y[2];
   M(1, 3) = 0.0;
   M(2, 0) = z[0];
   M(2, 1) = z[1];
   M(2, 2) = z[2];
   M(2, 3) = 0.0;
   M(3, 0) = 0.0;
   M(3, 1) = 0.0;
   M(3, 2) = 0.0;
   M(3, 3) = 1.0;
#undef M
   glMultMatrixf(m);

   /* Translate Eye to Origin */
   glTranslatef(-eyex, -eyey, -eyez);

}

//senquack - 2/11 - disabled for now:
//Added by Albert to help with surface format conversion:
/*
 * SDL surface conversion to OpenGL texture formats
 *
 * Mattias Engdegård
 *
 * Use, modification and distribution of this source is allowed without
 * limitation, warranty or liability of any kind.
 */

/*
 * Convert a surface into one suitable as an OpenGL texture;
 * in RGBA format if want_alpha is nonzero, or in RGB format otherwise.
 * 
 * The surface may have a colourkey, which is then translated to an alpha
 * channel if RGBA is desired.
 *
 * Return the resulting texture, or NULL on error. The original surface is
 * always freed.
 */
SDL_Surface *conv_surf_gl(SDL_Surface *s, int want_alpha)
{
    Uint32 rmask, gmask, bmask, amask;
    SDL_Surface *conv;
    int bpp = want_alpha ? 32 : 24;

    /* SDL interprets each pixel as a 24 or 32-bit number, so our
       masks must depend on the endianness (byte order) of the
       machine. */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000 >> (32 - bpp);
    gmask = 0x00ff0000 >> (32 - bpp);
    bmask = 0x0000ff00 >> (32 - bpp);
    amask = 0x000000ff >> (32 - bpp);
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = want_alpha ? 0xff000000 : 0;
#endif

    /* check if the surface happens to be in the right format */
    if(s->format->BitsPerPixel == bpp
       && s->format->Rmask == rmask
       && s->format->Gmask == gmask
       && s->format->Bmask == bmask
       && s->format->Amask == amask
       && !(s->flags & SDL_SRCCOLORKEY)) {
	/* no conversion needed */
	return s;
    }

    /* wrong format, conversion necessary */

    /* SDL surfaces are created with lines padded to start at 32-bit boundaries
       which suits OpenGL well (as long as GL_UNPACK_ALIGNMENT remains
       unchanged from its initial value of 4) */
    conv = SDL_CreateRGBSurface(SDL_SWSURFACE, s->w, s->h, bpp,
				rmask, gmask, bmask, amask);
    if(!conv) {
	SDL_FreeSurface(conv);
	return NULL;
    }

    if(want_alpha) {
	/* SDL sets the SDL_SRCALPHA flag on all surfaces with an
	   alpha channel. We need to clear that flag for the copy,
	   since SDL would attempt to alpha-blend our image otherwise */
	SDL_SetAlpha(s, 0, 255);
    }

    /*
     * Do the conversion. If the source surface has a colourkey, then it
     * will be used in the blit. We use the fact that newly created software
     * surfaces are zero-filled, so the pixels not blitted will remain
     * transparent.
     */
    if(SDL_BlitSurface(s, NULL, conv, NULL) < 0) {
	/* blit error */
	SDL_FreeSurface(conv);
	conv = NULL;
    }
    SDL_FreeSurface(s);

    return conv;
}

//senquack - 2/11 - disabled for now:
/*
 * A sample use of conv_surf_gl():
 *
 * Load an image from a file, and convert it to RGB or RGBA format,
 * depending on the image.
 *
 * Return the resulting surface, or NULL on error
 */
SDL_Surface *load_gl_texture(char *file)
{
    SDL_Surface *s = IMG_Load(file);
    if(!s)
	   return NULL;
    return conv_surf_gl(s, s->format->Amask || (s->flags & SDL_SRCCOLORKEY));
}


