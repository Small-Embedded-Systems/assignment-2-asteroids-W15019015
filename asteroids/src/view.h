/* Asteroids View */

/*
view.h declares the function prototype for void draw(void); so it can be used in
main. It also has any other functions needed elsewhere (such as setting up the
double-buffering).

*/

/* support double buffering */

void init_DBuffer(void);
void swap_DBuffer(void);

void draw(void);
