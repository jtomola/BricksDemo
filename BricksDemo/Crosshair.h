#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include "Vect.h"

class Crosshair
{
public:
	Crosshair();

	void Draw();

public:
	Vect position;
	Vect scale;
	Vect color;
};

#endif