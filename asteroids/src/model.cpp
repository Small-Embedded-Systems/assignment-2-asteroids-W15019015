/* Asteroids model */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "model.h"
#include "utils.h"
#include "asteroids.h"
/*
	model.cpp handles updating the game state, moving the objects, and handling collisions.
*/

static void wrapScreenShip(void);
static void wrapScreenAsteroid(asteroid *asteroid);
static void wrapScreenBullet(bullet *shot);

static void updateShip(void);
static void updateBullets(bullet *shot);
static void updateAsteroids(asteroid *asteroid);

static void destroyBullet(bullet *s);
static void destroyAsteroid(asteroid *ast);

//when the players shield is active, deplete it
static void shieldHandler(void);


//when an asteroid is struck by a bullet, a new asteroid is created and the current one shrinks and is modified
static void shotAsteroid(asteroid *ast);

static bool checkBulletCollision(asteroid *ast);

void physics(void){	
	if (!paused&&inPlay) {
		updateShip();	
		updateAsteroids(activeAsteroid);
		updateBullets(activeShot);
		shieldHandler();
	}
}

void shieldHandler(void) {
if (shieldActive) {
	shieldCharge--;
	if (shieldCharge<=0)
		shieldActive=false;

}else if (shieldCharge<100)
	shieldCharge++;
}
////////////////////////////////////////////////////////////////////////
//Screen Wrapping
////////////////////////////////////////////////////////////////////////

//when elements leave one side of the playing area, move them to the other

void wrapScreenShip() {
	if (ship.p.x<GAME_L-SHIP_RADIUS)
	ship.p.x=GAME_R+SHIP_RADIUS;
	if (ship.p.x>GAME_R+SHIP_RADIUS)
	ship.p.x=GAME_L-SHIP_RADIUS;
	
	if (ship.p.y<GAME_T-SHIP_RADIUS)
	ship.p.y=GAME_B+SHIP_RADIUS;
	if (ship.p.y>GAME_B+SHIP_RADIUS)
	ship.p.y=GAME_T-SHIP_RADIUS;
}

void wrapScreenBullet(bullet *shot) {
	if (shot->p.x<GAME_L)
	shot->p.x=GAME_R;
	if (shot->p.x>GAME_R)
	shot->p.x=GAME_L;
	
	if (shot->p.y<GAME_T)
	shot->p.y=GAME_B;
	if (shot->p.y>GAME_B)
	shot->p.y=GAME_T;
}

//repositioning of asteroids once they leave the screen, 
void wrapScreenAsteroid(asteroid *ast) {
	if (ast->p.x<GAME_L-(ast->radius))
	ast->p.x=GAME_R+ast->radius;
	if (ast->p.x>GAME_R+(ast->radius))
	ast->p.x=GAME_L-ast->radius;
	
	if (ast->p.y<GAME_T-(ast->radius))
	ast->p.y=GAME_B+ast->radius;
	if (ast->p.y>GAME_B+(ast->radius))
	ast->p.y=GAME_T-ast->radius;
}	


/////////////////////////////////////////////////////////////
//Object Movement
/////////////////////////////////////////////////////////////

//general ship movement
void updateShip() {
	wrapScreenShip();
	//move the ship every tick
	ship.p.x +=	ship.v.x;
	ship.p.y +=	ship.v.y;
	
	if (ship.angle>359)
	ship.angle=ship.angle-360;
	if (ship.angle<0)
	ship.angle=ship.angle+360;
}

void updateAsteroids(struct asteroid *ast) {
	for (;ast;ast=ast->next) {		
		if (ast->size>0) {
			wrapScreenAsteroid(ast);
			ast->p.x+= ast->v.x;
			ast->p.y+= ast->v.y;
			ast->angle+=ast->rot*0.25;
			
			if (ast->angle>359)
				ast->angle=ast->angle-360;
			if (ast->angle<0)
				ast->angle=ast->angle+360;

			if (!shieldActive)
				if (ship.p.x > ast->p.x-ast->radius && ship.p.x < ast->p.x+ast->radius)
					if (ship.p.y > ast->p.y-ast->radius && ship.p.y < ast->p.y+ast->radius) {
						loseLife();
					}
			if (checkBulletCollision(ast)) { //check to see if a bullet collides with an asteroid
				shotAsteroid(ast); //if so, trigger action for when it has
			}
		}
				if (ast->size>0)
		if (ast->next->size==-1)
			destroyAsteroid(ast);
	}
}

///////////////////////////////////////////////////////
//Object interactions
///////////////////////////////////////////////////////
//asteroid has been shot, so create a new asteroid and adjust the current
void shotAsteroid(asteroid *ast) {
	score+=ast->size;
	if (ast->size>1) {
		createAsteroid(ast->p.x,ast->p.y,ast->size-1);
		ast->size= ast->size--;
		ast->rot=randrange(-4,5);
		ast->angle= randrange(0,361);
		
		ast->v.x= cos(radians(ast->angle))/(ast->size);
		ast->v.y= sin(radians(ast->angle))/(ast->size);
		
		//radius is used for collisions
		ast->radius=ast->size*10;
		}else if (ast->size==1){
			asteroidCount--;
		ast->size=-1;
	}
}

bool checkBulletCollision(asteroid *ast) {
	struct bullet *shot=activeShot;
	for (;shot;shot=shot->next) {
		if (ast->size>0) {
			if (shot->p.x > ast->p.x-ast->radius && shot->p.x < ast->p.x+ast->radius)
			if (shot->p.y > ast->p.y-ast->radius && shot->p.y < ast->p.y+ast->radius) {
				if (ast->size>0) {
					if (shot->ttl>0) {
						shot->ttl=0;
						return true;
					}	
				}
			}
		}
	}
	return false;
}

void updateBullets(struct bullet *shot) {
	for (;shot;shot=shot->next) {
		if (shot->ttl>0){
			wrapScreenBullet(shot);
			shot->p.x+=shot->v.x;
			shot->p.y+=shot->v.y;
			shot->ttl--;
			if(shot->next->ttl<=0)
			destroyBullet(shot);
		}
	}
}

void destroyBullet(struct bullet *shot) {
	struct bullet *expired = shot->next;
	if (shot->next)//check if a node exists after this, 
		shot->next = shot->next->next;//if so, change the link to the node after the next
	releaseBullet(expired);
}

void destroyAsteroid(struct asteroid *ast) {
	struct asteroid *expired = ast->next;
if (ast->next)
	ast->next = ast->next->next;
	releaseAsteroid(expired);
}