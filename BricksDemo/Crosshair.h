#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include "Vect.h"

// Just a simple class to represent our crosshair
// One for horizontal and one for vertical part
// Uses the standard cube vertex buffer created in Demo class
class Crosshair
{
public:
	Crosshair();

	void Draw();

	Vect position;
	Vect scale;
	Vect color;
};

#endif