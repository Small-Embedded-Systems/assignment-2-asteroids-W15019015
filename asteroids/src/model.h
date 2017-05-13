/* Asteroids Model */
/*
model.h exposes the parts of the model needed elsewhere. Such as the fire function
called by the controller.
*/

struct point {
    float x,y;
};
typedef struct point coordinate_t;
typedef struct point vector_t;

enum gameStats {SHIP_TURNSPEED=10, SHIP_RADIUS=10, BULLET_SPEED=3,BULLET_TTL=200};

enum heapValues {BULLETS_MAX=5,ASTEROIDS_MAX=25};

enum size { //for astroids
	DESTROYED=0, SMALL=1, MEDIUM=2, LARGE=3
	};

typedef struct asteroid{
	coordinate_t p; //point x, point y
	coordinate_t v; //velocity x, velocity y
	double angle; //current angle of movement
	int rot; //rotational speed, visual effect only
	int speed; //current travelling speed
	int size; //0=destroyed
	int radius; //for collisions
  struct asteroid *next;
}asteroid_t;

extern void releaseAsteroid(asteroid *asteroid);

extern asteroid_t *allocAsteroid(void);
extern struct asteroid *activeAsteroid;

typedef struct ship {
coordinate_t p;//central point, p.x, p.y
vector_t v;//velocity, v.x, v.y
	double angle;
	int colour;
	//acceleration value, increases when input held, 
	//decreases when released, caps at a certain level.	
}ship_t;

extern ship_t ship;

typedef struct bullet {
coordinate_t p;//central point, p.x, p.y
vector_t v;//velocity, v.x, v.y
	int ttl; //time to live, dies after time met
	struct bullet *next;
}bullet_t;

extern void releaseBullet(bullet *shot);
extern bullet_t *allocBullet(void);
extern struct bullet *activeShot;

void physics(void);

void initialiseHeaps(void);
