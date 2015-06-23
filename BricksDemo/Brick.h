#ifndef BRICK_H
#define BRICK_H

#include "Vect.h"
#include "Matrix.h"
#include "Quat.h"

class Brick
{
public:
	Brick();

	void Draw();
	void Update(const float elapsedTime);
	void CalculateDerivedData();

	Matrix              transformMatrix;
	Matrix              inverseInertiaTensor;
	Matrix              inverseInertiaTensorWorld;

	Vect                position;
	Vect                velocity;
	Vect                acceleration;
	Quat                rotation;
	Vect                angVelocity;
	Vect                angAcceleration;

	Vect				scale;
	Vect				color;

	float               inverseMass;
};

#endif