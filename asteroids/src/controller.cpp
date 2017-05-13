/* Controller */

/*
	controller.cpp has the code for the user interface (the input side). This polls the joystick
	and sets values, or calls functions as needed.
*/

/* C libraries */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* hardware platform libraries */
#include <mbed.h>

/* asteroids */
#include "model.h"
#include "asteroids.h"
#include "utils.h"

/* Joystick 5-way switch*/
static DigitalIn joystick[] = {P5_0, P5_1, P5_4, P5_2, P5_3};
enum position {LEFT,DOWN,RIGHT,UP,CENTRE};

bool stickPressed(position button);
bool stickPressedAndReleased(position button);

//partial implementation of assembly joystick
//extern "C" bool joyPressed(int);  

void controls(void){
	if (paused) {
		if (stickPressedAndReleased(CENTRE))
		paused=UNPAUSED;
		}else if (!inPlay){
		if (stickPressedAndReleased(CENTRE))
			inPlay=true;
	}else{
		if (stickPressed(LEFT)) {
			ship.angle+=SHIP_TURNSPEED;
		}else 
		if (stickPressed(RIGHT)) {
			ship.angle-=SHIP_TURNSPEED;
		}
		if (stickPressed(UP)) {
			ship.v.x+=cos(radians(ship.angle))*0.1;
			ship.v.y+=sin(radians(ship.angle))*0.1;
		}else		
		if (stickPressedAndReleased(DOWN)) {
			if (shieldCharge==100)
				shieldActive=true;
//	 used for debugging asteroids
//			int size=randrange(1,6);
//			createAsteroid(ship.p.x,ship.p.y,1);
		}
		//shoot bullets when pressing the stick in
		if (stickPressedAndReleased(CENTRE)) {
			createBullet();
		}
	}
}

//in some situations the stick needs to be pressed and released instead
//of held, e.g. shooting bullets, activating shield
bool stickPressedAndReleased(position button) {
	bool result = false;
	uint32_t state;
	static uint32_t savedState[5] = {1,1,1,1,1};
	//initially all 1s: nothing pressed
	state = joystick[button].read();
	if ((savedState[button] == 0) && (state == 1)) {
		result = true;
	}
	savedState[button] = state;
	return result;
}

//for turning etc, action must be instantanious
bool stickPressed(position button) {
	bool result = false;
	if (joystick[button].read()==0) {
		result=true;		
	}
	return result;
}
