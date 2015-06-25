#ifndef BLOCK_H
#define BLOCK_H

#include "Vect.h"
#include "Matrix.h"
#include "Quat.h"

class Block
{
public:
	Block();

	void Draw();
	void Update(const float elapsedTime);
	void CalculateDerivedData();
	void CalcInertiaTensor();

	Matrix              transformMatrix;
	Matrix              inverseInertiaTensor;
	Matrix              inverseInertiaTensorWorld;

	Vect                position;
	Vect                velocity;
	Vect                acceleration;
	Quat                rotation;
	Vect                angVelocity;
	Vect                angAcceleration;
	Vect				force;
	Vect				torque;

	Vect				scale;
	Vect				color;

	float               inverseMass;
	bool				gravity;
	bool				active;
};


#endif