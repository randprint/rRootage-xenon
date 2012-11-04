/*
 * $Id: rr.c,v 1.4 2003/04/26 03:24:16 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * rRootage main routine.
 *
 * @version $Revision: 1.4 $
 */
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "rr.h"
#include "screen.h"
#include "vector.h"
#include "foe_mtd.h"
#include "brgmng_mtd.h"
#include "degutil.h"
#include "boss_mtd.h"
#include "ship.h"
#include "laser.h"
#include "frag.h"
#include "shot.h"
#include "background.h"
#include "soundmanager.h"
#include "attractmanager.h"
#ifdef XENON
#include <xetypes.h>
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <xenos/xenos.h>
#include <threads/threads.h>
//http remote screenshots
#include "httpd.h"
// for gdb
#include <threads/gdb.h>
#include <network/network.h>
#include <libfat/fat.h>
#include <usb/usbmain.h>
#include <diskio/ata.h>
#include <console/console.h>
#include <xenon_smc/xenon_smc.h>
#include <xenon_soc/xenon_power.h>
#include <sys/iosupport.h>
#include <xenon_sound/sound.h>
#endif
static int noSound = 0;

// Initialize and load preference.
static void initFirst() {
  time_t timer;
  time(&timer);
  srand(timer);

  loadPreference();
  initBarragemanager();
  initAttractManager();
  if ( !noSound ) initSound();
  initGameStateFirst();
  //printf("initFirst: Done\n");
}

// Quit and save preference.
void quitLast() {
  if ( !noSound ) closeSound();
  savePreference();
  closeFoes();
  closeBarragemanager();
  closeSDL();
  SDL_Quit();
  exit(1);
}

int status;

void initTitleStage(int stg) {
  initFoes();
  initStageState(stg);
}

void initTitle() {
  int stg;
  status = TITLE;

  stg = initTitleAtr();
  initBoss();
  initShip();
  initLasers();
  initFrags();
  initShots();
  initBackground(0);
  initTitleStage(stg);
  left = -1;
}

void initGame(int stg) {
  int sn;
  status = IN_GAME;
	//printf("initGame1\n");
  initBoss();
	//printf("initGame2\n");
  initFoes();
	//printf("initGame3\n");
  initShip();
	//printf("initGame4\n");
  initLasers();
	//printf("initGame5\n");
  initFrags();
	//printf("initGame6\n");
  initShots();
	//printf("initGame7\n");
  initGameState(stg);
	//printf("initGame8\n");
  sn = stg%SAME_RANK_STAGE_NUM;
	//printf("initGame9\n");
  initBackground(sn);
	//printf("initGame10\n");
  if ( sn == SAME_RANK_STAGE_NUM-1 ) {
	//printf("initGame11\n");
    playMusic(rand()%(SAME_RANK_STAGE_NUM-1));
	//printf("initGame12\n");
  } else {
	//printf("initGame13\n");
    playMusic(sn);
	//printf("initGame14\n");
  }
}

void initGameover() {
  status = GAMEOVER;
  initGameoverAtr();
}

static void move() {
  switch ( status ) {
  case TITLE:
//    printf("move(): TITLE\n");
    moveTitleMenu();
    moveBoss();
    moveFoes();
    moveBackground();
    break;
  case IN_GAME:
  case STAGE_CLEAR:
 //   printf("move(): move in game\n");
    moveShip();
    moveBoss();
    moveLasers();
    moveShots();
    moveFoes();
    moveFrags();
    moveBackground();
    break;
  case GAMEOVER:
 //   printf("move(): move GAMEOVER\n");
    moveGameover();
    moveBoss();
    moveFoes();
    moveFrags();
    moveBackground();
    break;
  case PAUSE:
  //  printf("move(): move PAUSE\n");
    movePause();
    break;
  }
  moveScreenShake();
}



static void draw() {
  switch ( status ) {
  case TITLE:
  	//printf("draw(): Drawing TITLE\n");
  	//printf("draw(): Drawing TITLE - drawBackground\n");
    drawBackground();
   // printf("draw(): Drawing TITLE - drawBoss\n");
    drawBoss();
  //  printf("draw(): Drawing TITLE - drawBulletsWake\n");
    drawBulletsWake();
    
    //TODO: FIX CRASH IN drawBulletsWake() !!!!!!!!!!!!!!!!! **********
    // When drawBulletsWake used (or buttons pushed), it crashes?
    
    
   // printf("draw(): Drawing TITLE - drawBullets\n");
    drawBullets();
  //  printf("draw(): Drawing TITLE - startDrawBoards\n");
    startDrawBoards();
 //   printf("draw(): Drawing TITLE - drawSideBoards\n");
    drawSideBoards();
   // printf("draw(): Drawing TITLE - drawTitle\n");
    drawTitle();
   // printf("draw(): Drawing TITLE - endDrawBoards\n");
    endDrawBoards();
  //  printf("draw(): Drawing TITLE - drawTestPoly\n");
//    drawTestPoly();
    break;
  case IN_GAME:
  case STAGE_CLEAR:
	 //senquack
  	//printf("draw(): Drawing STAGE_CLEAR\n");
    drawBackground();
	 // NOTE - senquack: just drawShip and drawBullets here fixes freezes, enabling drawBoss
	 //		causes freezing very quickly
    drawBoss();
    drawLasers();
    drawShots();
    drawBulletsWake();
    drawFrags();
    drawShip();
    drawBullets();
    startDrawBoards();
    drawSideBoards();
    drawBossState();
    endDrawBoards();
    break;
  case GAMEOVER:
	 //senquack
  	//printf("draw(): Drawing GAMEOVER\n");
    drawBackground();
    drawBoss();
    drawBulletsWake();
    drawFrags();
    drawBullets();
    startDrawBoards();
    drawSideBoards();
    drawGameover();
    endDrawBoards();
    break;
  case PAUSE:
	 //senquack
  	//printf("draw(): Drawing PAUSE\n");
    drawBackground();
    drawBoss();
    drawLasers();
    drawShots();
    drawBulletsWake();
    drawFrags();
    drawShip();
    drawBullets();
    startDrawBoards();
    drawSideBoards();
    drawBossState();
    drawPause();
    endDrawBoards();
    break;
  }
}

static int accframe = 0;

static void usage(char *argv0) {
  fprintf(stderr, "Usage: %s [-laser] [-lowres] [-nosound] [-reverse] [-nowait] [-accframe]\n", argv0);
}

static void parseArgs(int argc, char *argv[]) {
  int i;
  for ( i=1 ; i<argc ; i++ ) {
    if ( strcmp(argv[i], "-lowres") == 0 ) {
      lowres = 1;
    } else if ( strcmp(argv[i], "-nosound") == 0 ) {
      noSound = 1;
//    } else if ( strcmp(argv[i], "-window") == 0 ) {
//      windowMode = 1;
    } else if ( strcmp(argv[i], "-reverse") == 0 ) {
      buttonReversed = 1;
    }
    /* else if ( (strcmp(argv[i], "-brightness") == 0) && argv[i+1] ) {
      i++;
      brightness = (int)atoi(argv[i]);
      if ( brightness < 0 || brightness > 256 ) {
	brightness = DEFAULT_BRIGHTNESS;
      }
      }*/ 
    else if ( strcmp(argv[i], "-nowait") == 0 ) {
      nowait = 1;
    } else if ( strcmp(argv[i], "-accframe") == 0 ) {
      accframe = 1;
    } else if ( strcmp(argv[i], "-laser") == 0 ) {
      laserOnByDefault = 1;
    } else {
      usage(argv[0]);
      exit(1);
    }
  }
}
unsigned int bail(unsigned int context)
{
	puts("Core 0: Shuting down threads\n");
	threading_shutdown();
	puts("Core 0: Exit");
	exit(0);

	return 0;
}

//Isn't in the header, adding so we can ipi core 0 specifically
extern void ipi_send_packet(thread_ipi_proc entrypoint,
        unsigned int context,
        char processors, unsigned volatile int *count);

extern void request_dump();

void bail_thread()
{
	PTHREAD pthr = thread_get_current();
	thread_set_name(pthr, "crit");
	
	// If threads/gdb are fucked, disable interrupts
	unsigned int msr = thread_disable_interrupts();

	while(1)
	{
		
		char c = getch();
		printf("got char\n");
		if(c == 'x')
		{
			printf("Sending ipi\n");
			unsigned int count;
			ipi_send_packet(bail, 0, 1, &count);
		}
		if(c == 'd')
		{
			
			int i;
			for(i = 0; i < MAX_THREAD_COUNT; i++)
			{
				PTHREAD pthr = thread_get_pool(i);
				if(pthr->Valid){
					if(!pthr->Name)
						printf("Thread %i Dump, cpu:%i, sc:%i, st:%lli, wfm:%i, ls:%lli ,name:, Iar:%llu", pthr->ThreadId, pthr->ThisProcessor->CurrentProcessor, pthr->SuspendCount, pthr->SleepTime, pthr->WaitingForMutex, _millisecond_clock_time - pthr->LastTime, pthr->Context.Iar);
					else
						printf("Thread %i Dump, cpu:%i, sc:%i, st:%lli, wfm:%i, ls:%lli ,name:%s, Iar:%llu", pthr->ThreadId, pthr->ThisProcessor->CurrentProcessor, pthr->SuspendCount, pthr->SleepTime, pthr->WaitingForMutex, _millisecond_clock_time - pthr->LastTime, pthr->Name, pthr->Context.Iar);
					printf(",prio:%i,pb:%i, lr:%llu", pthr->Priority, pthr->PriorityBoost, _millisecond_clock_time - pthr->ReadyTime);
					printf(",mp:%i, tt:%i\n", pthr->MaxPriorityBoost, pthr->ThreadTerminated);
					printf("current time:%lli\n", _millisecond_clock_time);

				}
			}
			//request_dump();
			
		}
		//udelay(10);
	}
}

void startbailthread()
{
	PTHREAD bailthread = thread_create((void*)bail_thread, 0, 0, THREAD_FLAG_CREATE_SUSPENDED);
	thread_set_processor(bailthread, 3);
	thread_resume(bailthread);
}

int interval = INTERVAL_BASE;
int tick = 0;
static int pPrsd = 1;

int main(int argc, char *argv[]) {

  int done = 0;

  long prvTickCount = 0;
  int i;
  SDL_Event event;
  long nowTick;
  int frame;


	//xenon
	xenon_make_it_faster(XENON_SPEED_FULL);
	xenos_init(VIDEO_MODE_AUTO);
	threading_init();
//	for GDB
	network_init();
	gdb_init();
	//for debug on screen with no uart
	//console_init();
    xenon_sound_init();
    
	// xenon usb	
	usb_init();
	xenon_ata_init();
	usb_do_poll();
	fatInitDefault();
//debug shite
startbailthread();
http_output_start();

  parseArgs(argc, argv);

  initDegutil();
  printf("Init SDL\n");
  initSDL();
  printf("initFirst\n");
  initFirst();
  printf("initTitle\n");
  initTitle();

  //senquack
//printf("Init title done\n");
//fflush(stdout);

  while ( !done ) {
//printf("Starting to poll\n");
    SDL_PollEvent(&event);
//printf("poll done\n");
	//senquack - Quitting is handled from the main menu.  The quit button merely exits to the
	//				main menu and that logic is handled elsewhere now.
	if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_START)) {
		if ( !pPrsd ) {
			if ( status == IN_GAME ) {
				status = PAUSE;
			} else if ( status == PAUSE ) {
				status = IN_GAME;
			}
		}
		pPrsd = 1;
	} else {
		pPrsd = 0;
	}

	//senquack
	if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT) && status == IN_GAME) {
		initGameover();
	}
	 
    nowTick = SDL_GetTicks();
    frame = (int)(nowTick-prvTickCount) / interval;
	if ( frame <= 0 ) {
		frame = 1;
		//SDL_Delay(prvTickCount+interval-nowTick);
		udelay(prvTickCount+interval-nowTick);
		if ( accframe ) {
			prvTickCount = SDL_GetTicks();
		} else {
			prvTickCount += interval;
		}
	} else if ( frame > 5 ) {
			frame = 5;
			prvTickCount = nowTick;
	} else {
			prvTickCount += frame*interval;
	}
	for ( i=0 ; i<frame ; i++ ) {
		move();
		tick++;
	}


//printf("Starting to draw\n");
//fflush(stdout);

    drawGLSceneStart();
//printf("drawGLSceneStart done\n");
    draw();
//printf("draw done\n");    
    drawGLSceneEnd();
//printf("drawGLSceneEnd done\n");    
    swapGLScene();
//printf("swapGLScene done\n");    

  }
  quitLast();
  return 0;
}
