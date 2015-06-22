#ifndef BULLET_H
#define BULLET_H

#include "Vect.h"

class Bullet
{
public:
	Bullet();

	void Draw();
	void Update(const float timeIn);

public:
	Vect pos;
	Vect scale;
	Vect color;
	Vect velocity;
	bool active;
};

#endif