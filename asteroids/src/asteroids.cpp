/*
	asteroids.cpp The game, it includes main and declares any global variables needed
	for the game state.
*/

/* C libraries */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

/* hardware platform libraries */
#include <display.h>
#include <mbed.h>

/* Main game elements */
#include "model.h"
#include "view.h"
#include "controller.h"
#include "utils.h"
#include "asteroids.h"

/*Prototype methods*/
static void initialiseGame(void);
static void initialiseShip(void);
static void initialiseHeaps(void);
static void initialiseBulletHeap(void);
static void initialiseAsteroidHeap(void);
static void initialiseResources(void);

static void completeLevel(void);

static void removeAsteroids(void);
static void removeBullets(void);

static void time();

/* Game state */

int 	paused;					//is the game paused? (after life lost or level complete)
bool 	inPlay=false;		//is the game currently in play? (not on title screen)
int 	level;					//current level of the game, asteroids get bigger every level
int   lives;					//how many lives does the player have?

uint16_t 	elapsedTime;
uint16_t		bestTime;
uint16_t   score;
uint16_t 	highScore;

int 	asteroidCount;	//how many asteroids are in play
bool 	shieldActive;		//is the shield on?
int		shieldCharge;		//the current power left on shield

bullet *activeShot=NULL;
asteroid *activeAsteroid=NULL;

static float const Dt = 0.01f;

static bullet_t bullets[BULLETS_MAX];
static bullet_t *freeShot;
static asteroid_t asteroids[ASTEROIDS_MAX];
static asteroid_t *freeAsteroid;

Ticker model, view, controller, seconds;
ship_t ship;

int main() {
//initialise variables for game function
	initialiseHeaps();	
	initialiseResources();
	initialiseGame();
	initialiseShip();
	
	while(true) {
		if (inPlay) {
		if (asteroidCount<=0)  
			completeLevel();
		}
	}
}

//initialise ship for game
void initialiseShip() {
	shieldCharge=100;
	ship.angle = randrange(0,361);
	ship.v.x=0;
	ship.v.y=0;
	ship.p.x=SCREEN_R/2+GAME_L;
	ship.p.y=SCREEN_B/1.5;
}

//initialising various resources, renderer, controller, etc
void initialiseResources() {
	init_DBuffer();
	draw();
	view.attach( draw, 0.025);
	model.attach( physics, Dt);
	controller.attach( controls, 0.05);
	seconds.attach( time, 1);
}

//set values for when a game is started
void initialiseGame() {
	paused=GAME_START;
	shieldCharge=100;
	shieldActive=true;
	elapsedTime=0;
	level = 0;
	score = 0;
	lives = 5;
}

//controls ingame timer, but only when game is not paused
void time() {
	if (!paused&&inPlay)
	elapsedTime++;
}

void completeLevel() {
if (level>0)
	paused=LEVEL_COMPLETE;
	removeBullets();
	level++;
	//created asteroid size is based on level, gets larger every level
	createAsteroid(randrange(GAME_L,GAME_R),randrange(GAME_T,GAME_B),level);
	shieldCharge=100;	//dont want player to die if they happen to be under the asteroid
	shieldActive=true; //so charge then activate shield
}

void loseLife() {
	removeBullets();
	paused=LIFE_LOST;
	initialiseShip();
	lives--;
	shieldActive=true;
	if (lives<0) {
		if (elapsedTime>bestTime)
			bestTime=elapsedTime;
		if (score>highScore)
			highScore=score;
		inPlay=false;
		removeAsteroids();
		removeBullets();
		initialiseGame();
	}
}

void createAsteroid(float newX,float newY,int newSize) {
	struct asteroid *ast = allocAsteroid();
	if (ast) {
		ast->next = activeAsteroid;
		activeAsteroid=ast;

		ast->p.x=newX;
		ast->p.y=newY;
		
		ast->rot=randrange(-4,5);
		ast->size= newSize;
		ast->angle= randrange(0,361);
		
		ast->v.x= cos(radians(ast->angle))/(ast->size);
		ast->v.y= sin(radians(ast->angle))/(ast->size);
	
		//radius is used for collisions
		ast->radius=ast->size*11;
		asteroidCount++;
	}
}

void createBullet() {	
	struct bullet *shot = allocBullet();
	if (shot) {
		shot->next = activeShot;
		activeShot=shot;
		shot->p.x=ship.p.x;
		shot->p.y=ship.p.y;
		shot->v.x=ship.v.x+cos(radians(ship.angle));
		shot->v.y=ship.v.y+sin(radians(ship.angle));
		shot->ttl=BULLET_TTL;//how long the bullet lives
	}
}
////////////////////////////////////////////////////////
//clear objects
////////////////////////////////////////////////////////
void removeAsteroids() {
struct asteroid *ast=activeAsteroid;
	for (;ast;ast=ast->next) {
		ast->size=-1;
	}
	asteroidCount=0;
}
void removeBullets() {
struct bullet *shot=activeShot;
	for (;shot;shot=shot->next) {
		shot->ttl=0;
	}
}
///////////////////////////////////////////////////////
//initialising heaps
///////////////////////////////////////////////////////
void initialiseHeaps(void) {
	initialiseBulletHeap();
	initialiseAsteroidHeap();	
}

void initialiseBulletHeap(void) {
    int i;
    for( i=0 ; i<(BULLETS_MAX-1) ; i++) {
			bullets[i].next = &bullets[i+1];
	}
    bullets[i].next = NULL; //last node has no next
    freeShot = &bullets[0];
}

void initialiseAsteroidHeap(void) {
	int i;
	for( i=0; i<(ASTEROIDS_MAX-1); i++) {
		asteroids[i].next = &asteroids[i+1];
	}
		asteroids[i].next = NULL; //connect last node to the first
    freeAsteroid = &asteroids[0];
}


//////////////////////////
//Bullet Nodes
//////////////////////////
bullet_t *allocBullet(void) {
    bullet_t *shot = NULL;
    if (freeShot) {
        shot = freeShot;
		freeShot = freeShot->next;
	}
    return shot;
}

void releaseBullet(bullet_t *shot) {
	shot->next = freeShot;
	freeShot = shot;
}


/////////////////////////////////
//Asteroid Nodes
/////////////////////////////////
asteroid *allocAsteroid(void) {
	asteroid *ast = NULL;
	if (freeAsteroid) {
		ast = freeAsteroid;//the asteroid is the element within the array/list
		freeAsteroid = freeAsteroid->next; //set the free asteroid to the next in the list
	}
	return ast;
}

void releaseAsteroid(asteroid *ast) {
	ast->next = freeAsteroid;
	freeAsteroid = ast;
}
