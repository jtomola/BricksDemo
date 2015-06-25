#ifndef PHYSICS_CONTACT_H
#define PHYSICS_CONTACT_H

#include "Vect.h"
#include "Matrix.h"

class Block;

class PhysicsContact
{
public:
    PhysicsContact();
    ~PhysicsContact();

	void CalculateDesiredDeltaVelocity(const float timeIn);
    void ChangePosition();
    void ChangeVelocity();
	void Reset();
    void CalculateBasis();
    void CalculateData(const float timeIn);
    Vect CalculateLocalVelocity(const int blockIndex, const float timeIn);

    Matrix              worldToContact;
    Matrix              contactToWorld;
    Vect                normal;
	Vect				contactPoint;
    Vect                contactVelocity;
    Vect                relPos[2];
    Block*		        blocks[2];
    float               desiredDeltaVelocity;
    float               restitution;
    float               penetration;
	float				velocityFromAcc;
};

#endif
