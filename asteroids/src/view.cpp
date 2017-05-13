/* Asteroids view*/
/*
	view.cpp Handles all the drawing of the screen. I can declare the display variables
	in here, as they aren't needed in other parts of the program
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

#include "asteroids.h"
#include "model.h"
#include "utils.h"
#include "images.h"

Display *graphics = Display::theDisplay();

static bool screenClear=false; 		//for rendering titlescreen, clears the screen once to prevent flickering
static uint8_t shieldEffect_Size; //shield grows and shrinks slightly when active, this controls that
static bool shieldEffect_Grow;	//is the shield growing or shrinking?

enum renderingSettings{
	//colours are an integer value so I can set them here
	GAME_BACKGROUND_COLOUR=rgb(0,51,102),UI_BACKGROUND_COLOUR=BLACK,UI_BORDER_COLOUR=WHITE, 
	TITLE_IMAGE_COLOUR=WHITE,TITLE_BACKGROUND_COLOUR=BLACK,
	UI_FONT_COLOUR=WHITE, PAUSE_FONT_COLOUR=CYAN,
	LIVES_IMAGE_COLOUR=RED,

	SHIELDBAR_CHARGING_COLOUR=YELLOW,SHIELDBAR_DEPLETING_COLOUR=RED,SHIELDBAR_FULL_COLOUR=GREEN,

  SHIELD_COLOUR1=CYAN, SHIELD_COLOUR2=GREEN, 
	
	SHIP_COLOUR=GREEN, ASTEROID_COLOUR=WHITE, BULLET_COLOUR=YELLOW,
//positioning of screen elements
	LIVES_Y_POS=10,SCORE_Y_POS=30,TIME_Y_POS=50,ROCKCOUNT_Y_POS=70,SHIELDCHARGE_Y_POS=90,
	PAUSE_X_POS=GAME_L*2,PAUSE_Y_POS=GAME_B/2,
	TITLE_X_POS=130,TITLE_Y_POS=70,

	//for shield throbbing visual effect
	SHIELD_MINSIZE=7,SHIELD_MAXSIZE=9
};

static void renderTitleScreen();
static void renderUI();
static void renderPaused();
static void renderShip ();
static void renderLives();
static void renderScore();
static void renderTime();
static void renderShield();
static void renderAsteroidCount();
static void renderAsteroid(asteroid *asteroid);
static void renderBullets(bullet *shot);
static void renderShieldCharge();

static const coordinate_t rockShape[] = {
	{-15,-5},{0,-15},{15,-5},{10,12},{-10,12}
};

static const coordinate_t shipShape[] = {
	{10,0}, {-5,5}, {-5,-5}
};

/* double buffering functions */
void init_DBuffer(void)
{   /* initialise the LCD driver to use second frame in buffer */
	uint16_t *bufferbase = graphics->getFb();
	uint16_t *nextbuffer = bufferbase+ (480*272);
	LPC_LCD->UPBASE = (uint32_t)nextbuffer;
}

void swap_DBuffer(void)
{   /* swaps frames used by the LCD driver and the graphics object */
	uint16_t *buffer = graphics->getFb();
	graphics->setFb( (uint16_t*) LPC_LCD->UPBASE);
	LPC_LCD->UPBASE = (uint32_t)buffer;
}

void draw(void){
	if (!inPlay) {
			renderTitleScreen();
		}else{
		graphics->fillScreen(GAME_BACKGROUND_COLOUR);
		if (paused) //if the game is paused (any value not zero)
		renderPaused();
		renderShip(); 
		renderAsteroid(activeAsteroid);
		renderBullets(activeShot);
		if (shieldActive)
		renderShield();
		screenClear=false;		
		//need to draw over the top of game elements so UI is rendered last
		renderUI();
		}
		swap_DBuffer();
}

void renderTitleScreen(){
if (!screenClear)	{
	graphics->fillScreen(TITLE_BACKGROUND_COLOUR);
	//need to make sure both buffers are filled black to prevent flickering on titlescreen
	swap_DBuffer();
	graphics->fillScreen(TITLE_BACKGROUND_COLOUR);
	screenClear=true;
}
	graphics->setTextSize(2);
	if (bestTime>0) {
		graphics->setCursor(TITLE_X_POS, TITLE_Y_POS+asttitle_height+20);
		graphics->printf("Best Time: %u",bestTime);
	}
	if (highScore>0) {
		graphics->setCursor(TITLE_X_POS, TITLE_Y_POS+asttitle_height+40);
		graphics->printf("Highscore: %u",highScore);
	}
	graphics->drawBitmap(TITLE_X_POS, TITLE_Y_POS, asttitle_bits,
	asttitle_width, asttitle_height, TITLE_IMAGE_COLOUR);
	graphics->setCursor(TITLE_X_POS, TITLE_Y_POS-20);
	graphics->printf("Press joystick to play");
	graphics->setTextSize(1);
}

void renderUI(void) {
	graphics->fillRect(SCREEN_L,GAME_T,GAME_L,GAME_B,UI_BACKGROUND_COLOUR);
	graphics->drawLine(GAME_L,GAME_T,GAME_L,GAME_B,UI_BORDER_COLOUR);
	renderLives();
	renderAsteroidCount();		
	renderScore();
	renderTime();
	renderShieldCharge();
}

void renderLives() {
	graphics->setCursor(1, LIVES_Y_POS);
	graphics->printf("Lives");
	int i;
	for (i=0;i<lives;i++) {
		graphics->drawBitmap(i*(life_width-1), LIVES_Y_POS+10, life_bits,
		life_width, life_height, LIVES_IMAGE_COLOUR);
	}
//	graphics->setCursor(1, LIVES_Y_POS+10);
//	graphics->printf("%d",lives);
}

void renderScore () {
	graphics->setCursor(1, SCORE_Y_POS);
	graphics->printf("Score");
	
	graphics->setCursor(1, SCORE_Y_POS+10);
	graphics->printf("%d",score);
}

void renderPaused() {
	graphics->setCursor(PAUSE_X_POS, PAUSE_Y_POS);
	graphics->setTextSize(3);
	graphics->setTextColor(PAUSE_FONT_COLOUR);
	graphics->printf("Press Stick to resume");
	
	graphics->setCursor(PAUSE_X_POS, PAUSE_Y_POS-30);
	if (paused==LIFE_LOST)
		graphics->printf("Life lost!");
	if (paused==LEVEL_COMPLETE)
		graphics->printf("Level Complete!");
		
	graphics->setTextSize(1);
	graphics->setTextColor(UI_FONT_COLOUR);
}

void renderTime () {
	graphics->setCursor(1, TIME_Y_POS);
	graphics->printf("Time");
	
	graphics->setCursor(1, TIME_Y_POS+10);
	graphics->printf("%u",elapsedTime);	
}

void renderAsteroidCount() {
	graphics->setCursor(1, ROCKCOUNT_Y_POS);
	graphics->printf("Rocks");
	
	graphics->setCursor(1, ROCKCOUNT_Y_POS+10);
	graphics->printf("%d",asteroidCount);
}

void renderShieldCharge() {
	int colour;
	if (shieldActive)
	colour=SHIELDBAR_DEPLETING_COLOUR;
	else if (shieldCharge<100)
	colour=SHIELDBAR_CHARGING_COLOUR;
	else
	colour=SHIELDBAR_FULL_COLOUR;	
	graphics->setCursor(1, SHIELDCHARGE_Y_POS);
	graphics->printf("Shield");
	
	graphics->drawRect(1,SHIELDCHARGE_Y_POS+10,(GAME_L)-2,10,LIGHTGREEN);
	graphics->fillRect(2,SHIELDCHARGE_Y_POS+11,(shieldCharge*GAME_L/100)-4,8,colour);	
}

void renderShield() {
	if (shieldEffect_Size<=SHIELD_MINSIZE)
	shieldEffect_Grow=true;
	else if (shieldEffect_Size>=SHIELD_MAXSIZE)
	shieldEffect_Grow=false;
if (shieldEffect_Grow)
	shieldEffect_Size++;
	else
	shieldEffect_Size--;
	graphics->drawCircle(ship.p.x,ship.p.y,shieldEffect_Size,SHIELD_COLOUR1);
	graphics->drawCircle(ship.p.x,ship.p.y,shieldEffect_Size+1,SHIELD_COLOUR2);
}

void renderShip(){	
	double point1x=shipShape[0].x*cos(radians(ship.angle)) -  shipShape[0].y*sin(radians(ship.angle));
	double point1y=shipShape[0].x*sin(radians(ship.angle)) +  shipShape[0].y*cos(radians(ship.angle));
	
	double point2x=  shipShape[1].x*cos(radians(ship.angle)) - shipShape[1].y*sin(radians(ship.angle));
	double point2y=  shipShape[1].x*sin(radians(ship.angle)) + shipShape[1].y*cos(radians(ship.angle));
	
	double point3x= shipShape[2].x*cos(radians(ship.angle)) -  shipShape[2].y*sin(radians(ship.angle));
	double point3y= shipShape[2].x*sin(radians(ship.angle)) +  shipShape[2].y*cos(radians(ship.angle));
	
	graphics->drawTriangle(ship.p.x+point1x, ship.p.y+point1y, ship.p.x+point2x, ship.p.y+point2y, ship.p.x+point3x, ship.p.y+point3y, SHIP_COLOUR);	
}

void renderAsteroid(struct asteroid *asteroid){
	while (asteroid) {
		if (asteroid->size>=0) {	
			double point1x=rockShape[0].x*cos(radians(asteroid->angle)) -  rockShape[0].y*sin(radians(asteroid->angle));
			double point1y=rockShape[0].x*sin(radians(asteroid->angle)) +  rockShape[0].y*cos(radians(asteroid->angle));
			
			double point2x=rockShape[1].x*cos(radians(asteroid->angle)) -  rockShape[1].y*sin(radians(asteroid->angle));
			double point2y=rockShape[1].x*sin(radians(asteroid->angle)) +  rockShape[1].y*cos(radians(asteroid->angle));
			
			double point3x=rockShape[2].x*cos(radians(asteroid->angle)) -  rockShape[2].y*sin(radians(asteroid->angle));
			double point3y=rockShape[2].x*sin(radians(asteroid->angle)) +  rockShape[2].y*cos(radians(asteroid->angle));
			
			double point4x=rockShape[3].x*cos(radians(asteroid->angle)) -  rockShape[3].y*sin(radians(asteroid->angle));
			double point4y=rockShape[3].x*sin(radians(asteroid->angle)) +  rockShape[3].y*cos(radians(asteroid->angle));
			
			double point5x=rockShape[4].x*cos(radians(asteroid->angle)) -  rockShape[4].y*sin(radians(asteroid->angle));
			double point5y=rockShape[4].x*sin(radians(asteroid->angle)) +  rockShape[4].y*cos(radians(asteroid->angle));
			

			//for debugging purposes			
			//collision box
			//graphics->drawRect(	-asteroid->radius+asteroid->p.x,-asteroid->radius+asteroid->p.y,asteroid->radius<<1,asteroid->radius<<1,RED);
			//asteroid size value
			//graphics->setCursor(asteroid->p.x,asteroid->p.y);
			//graphics->printf("%d",asteroid->size);
			
			graphics->drawLine(asteroid->p.x+point1x*asteroid->size, asteroid->p.y+point1y*asteroid->size, asteroid->p.x+point2x*asteroid->size, asteroid->p.y+point2y*asteroid->size, ASTEROID_COLOUR);
			graphics->drawLine(asteroid->p.x+point2x*asteroid->size, asteroid->p.y+point2y*asteroid->size, asteroid->p.x+point3x*asteroid->size, asteroid->p.y+point3y*asteroid->size, ASTEROID_COLOUR);
			graphics->drawLine(asteroid->p.x+point3x*asteroid->size, asteroid->p.y+point3y*asteroid->size, asteroid->p.x+point4x*asteroid->size, asteroid->p.y+point4y*asteroid->size, ASTEROID_COLOUR);
			graphics->drawLine(asteroid->p.x+point4x*asteroid->size, asteroid->p.y+point4y*asteroid->size, asteroid->p.x+point5x*asteroid->size, asteroid->p.y+point5y*asteroid->size, ASTEROID_COLOUR);
			graphics->drawLine(asteroid->p.x+point5x*asteroid->size, asteroid->p.y+point5y*asteroid->size, asteroid->p.x+point1x*asteroid->size, asteroid->p.y+point1y*asteroid->size, ASTEROID_COLOUR);

		}
		asteroid = asteroid->next;
	}
}
void renderBullets(struct bullet *shot){
	while (shot) {
		if (shot->ttl>0)
		graphics->fillCircle(shot->p.x,shot->p.y,0.01*shot->ttl,BULLET_COLOUR); //if the bullet life is longer than 255, need to make sure it renders nicely
		shot = shot->next;
	}
}
