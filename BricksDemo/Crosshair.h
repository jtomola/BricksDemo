#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include "Vect.h"

class Crosshair
{
public:
	Crosshair();

	void Draw();
	void Update(const float timeIn);

public:
	Vect pos;
	Vect scale;
	Vect color;
};

#endif