#ifndef BLOCK_H
#define BLOCK_H

#include "Vect.h"
#include "Matrix.h"
#include "Quat.h"

enum MinMax
{
	MIN,
	MAX
};

class Block
{
public:
	Block();

	void Draw();
	void Update(const float elapsedTime);
	void CalculateDerivedData();
	void CalcInertiaTensor();
	bool PointInsideBlock(const Vect& pointIn);
	Vect GetCorner(const MinMax x, const MinMax y, const MinMax z);

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
	bool				gravityNow;
	bool				gravityEver;
	bool				active;
};


#endif