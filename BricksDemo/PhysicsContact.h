#ifndef PHYSICS_CONTACT_H
#define PHYSICS_CONTACT_H

#include "Vect.h"
#include "Matrix.h"

class Block;

// Class representing a collision
// Has knowledge to adjust the object's positions and velocities
class PhysicsContact
{
public:
	// Constructor and destructor
    PhysicsContact();
    ~PhysicsContact();

	void CalculateDesiredVelocityChange(const float timeIn);
    void ChangePosition();
    void ChangeVelocity();
	void Reset();
    void CalculateBasis();
    void CalculateData(const float timeIn);
    Vect CalculateLocalVelocity(const int blockIndex, const float timeIn);

	// Matrices to convert between world space and coordinate space local to contact
    Matrix              worldToContact;
    Matrix              contactToWorld;

	// Contact normal
    Vect                normal;

    // Contact point (world space)
	Vect				contactPoint;

	// Relative velocity at contact point
    Vect                contactVelocity;  

	// Position of contact point relative to each block
    Vect                relPos[2];

	// Pointers to blocks involved in collision
    Block*		        blocks[2];

	// Desired change in velocity
    float               desiredVelocityChange;

	// Restitution of collision (how much bounce)
    float               restitution;

	// Amount objects are penetrating
    float               penetration;

	// Velocity due to this frame's acceleration
	float				velocityFromAcc;
};

#endif
