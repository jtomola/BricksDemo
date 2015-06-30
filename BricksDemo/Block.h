#ifndef BLOCK_H
#define BLOCK_H

#include "Vect.h"
#include "Matrix.h"
#include "Quat.h"

// Used to specify corners of the block
enum MinMax
{
	MIN,
	MAX
};

class Block
{
public:
	// Default constructor
	Block();

	// Draw the block
	void Draw();

	// Update the physics of the block
	void Update(const float elapsedTime);

	// Calculate the necessary values for collisions each frame
	void CalculateDerivedData();

	// Calculate the inverse inertial tensor based on mass and box size
	void CalcInertiaTensor();

	// Test whether a point in world space is inside the block
	bool PointInsideBlock(const Vect& pointIn);

	// Get a corner of the block in world space
	Vect GetCorner(const MinMax x, const MinMax y, const MinMax z);

	// Matrices needed for physics, rotation, and collisions
	Matrix              transformMatrix;
	Matrix              inverseInertiaTensor;
	Matrix              inverseInertiaTensorWorld;

	// Values for our Newton physics - velocity/acceleration/force/etc.
	Vect                position;
	Vect                velocity;
	Vect                acceleration;
	Quat                rotation;
	Vect                angVelocity;
	Vect                angAcceleration;
	Vect				force;
	Vect				torque;

	// Scale and color for drawing
	Vect				scale;
	Vect				color;

	// Mass and gravity variables (gravity now and ever let us "sleep" a bit)
	float               inverseMass;
	bool				gravityNow;
	bool				gravityEver;
	bool				active;
};


#endif