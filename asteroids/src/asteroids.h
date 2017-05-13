/* Game state */
/*
asteroids.h declares common datatypes needed for the game, and makes the data
available to other modules via the extern type qualifier.
*/

//game has various paused states depending on what is happening ingame
enum pauseState{UNPAUSED=0,GAME_START=1,LIFE_LOST=2,LEVEL_COMPLETE=3};

//for rendering and gameplay object positioning purposes
enum area {
	SCREEN_L=0,SCREEN_R=480,SCREEN_T=0,SCREEN_B=272,
	GAME_T=0,GAME_B=SCREEN_B,GAME_L=40,GAME_R=480
};

extern uint16_t elapsedTime; 	// time this ship has been active
extern uint16_t bestTime;			// longest a player has stayed alive 
extern uint16_t score;       	// total score so far
extern uint16_t highScore;		// best score so far across all players
extern int shieldCharge;			// the current charge of the ship shield;
extern bool shieldActive; 		// is the shield on or off?
extern int   lives;        		// lives remaining
extern int asteroidCount;			// number of asteroids in play
extern int paused;						// paused state is active when a life is lost
extern bool inPlay;						// has the game been started?
//extern const float Dt; 				// Time step for physics, needed for consistent motion

void createBullet(void);
void loseLife(void);
void createAsteroid(float newX, float newY, int newSize);
