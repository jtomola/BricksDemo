#ifndef BRICK_H
#define BRICK_H

#include "Vect.h"

class Brick
{
public:
	Brick();

	void Draw();

public:
	Vect pos;
	Vect scale;
	Vect color;
};

#endif