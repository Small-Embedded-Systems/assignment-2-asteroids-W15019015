/*
controller.h exposes the parts of the controller needed elsewhere, mainly the bit needed
to set up the ticker object.
*/

void controls(void);

bool stickPressed(int button);
//bool buttonPressed(int button);
bool stickPressedAndReleased(int button);
