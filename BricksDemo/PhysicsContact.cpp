#include "PhysicsContact.h"
#include "Block.h"
#include <math.h>
#include <assert.h>

PhysicsContact::PhysicsContact()
    :   worldToContact(),
        contactToWorld(), 
        normal(0.0f, 0.0f, 0.0f),
		contactPoint(0.0f, 0.0f, 0.0f),
        contactVelocity(0.0f, 0.0f, 0.0f),
        desiredDeltaVelocity(0.0f),
        restitution(0.0f),
        penetration(0.0f),
		velocityFromAcc(0.0f)
{
};

PhysicsContact::~PhysicsContact()
{
};

void PhysicsContact::CalculateDesiredDeltaVelocity(const float timeIn)
{
	if (penetration == 0.0f)
	{
		return;
	}

    // Limit restitution if low velocity
    const float velocityLimit = 1.0f;
    float actingRestitution = this->restitution;

	Vect scaledNormal = timeIn * normal;
	
	// Need to remove velocity from this frame's acceleration.
	// Removing jitter for resting contacts
	velocityFromAcc = blocks[0]->acceleration.dot(-1.0f * scaledNormal) * timeIn;
	velocityFromAcc += blocks[0]->angAcceleration.cross(relPos[0])[2] * timeIn;

	velocityFromAcc -= blocks[1]->acceleration.dot(-1.0f * scaledNormal) * timeIn;
	velocityFromAcc -= blocks[1]->angAcceleration.cross(relPos[1])[2] * timeIn;

    if (abs(contactVelocity[2]) < velocityLimit)
    {
        actingRestitution = 0.0f;
    }

	if (contactVelocity[2] <= velocityFromAcc + 0.001f)
	{
        actingRestitution = 0.0f;
	}

    this->desiredDeltaVelocity = -contactVelocity[2] - actingRestitution * (contactVelocity[2] - velocityFromAcc);
};

void PhysicsContact::Reset()
{
    this->normal.set(0.0f, 0.0f, 0.0f);
    this->blocks[0] = 0;
    this->blocks[1] = 0;
    this->restitution = 0.25f;
    this->penetration = 0.0f;
};

void PhysicsContact::ChangeVelocity()
{
	Vect impulseContact;

	Vect velocityChange[2];
	Vect angVelocityChange[2];

	Vect& velocity0 = blocks[0]->velocity;
	Vect& velocity1 = blocks[1]->velocity;
	Vect& angVelocity0 = blocks[0]->angVelocity;
	Vect& angVelocity1 = blocks[1]->angVelocity;

	// Need to calculate the impulse now
	// We are assuming no friction here
	Vect deltaVelWorld = (relPos[0]).cross(normal);
	deltaVelWorld *= blocks[0]->inverseInertiaTensorWorld;
	deltaVelWorld = Vect(deltaVelWorld).cross( relPos[0] );

	// Now get change in contact coordinates in direction of normal
	float deltaVelocity = deltaVelWorld.dot(normal);

	// Add linear component
	deltaVelocity += blocks[0]->inverseMass;

	// Now do same for second body
	if (blocks[1] != 0 && blocks[1]->inverseMass != 0.0f)
	{
		deltaVelWorld = (relPos[1]).cross(normal);
		deltaVelWorld *= blocks[1]->inverseInertiaTensorWorld;
		deltaVelWorld = Vect(deltaVelWorld).cross( relPos[1] );

		// Add change due to rotation
		deltaVelocity +=  deltaVelWorld.dot(normal) ;

		// Add change due to linear motion
		deltaVelocity += blocks[1]->inverseMass;
	}

	// Return if no change needed
	if (desiredDeltaVelocity >= -0.01f && desiredDeltaVelocity <= 0.01f) return;

	// Calculate size of impulse
	impulseContact.set(0.0f, 0.0f, this->desiredDeltaVelocity / deltaVelocity);

    Vect impulse = impulseContact * contactToWorld;
	
    // Now split impulse into its linear and rotational components
    Vect impulsiveTorque = relPos[0].cross(impulse);
    angVelocityChange[0] = impulsiveTorque * blocks[0]->inverseInertiaTensorWorld;
    velocityChange[0].set(0.0f, 0.0f, 0.0f);
    velocityChange[0] += impulse * blocks[0]->inverseMass;
    
    // Apply changes to velocity and angular velocity
    blocks[0]->velocity += velocityChange[0];
	blocks[0]->angVelocity += angVelocityChange[0];

    // Now do same for body 1, if exists
    if (blocks[1] != 0 && blocks[1]->inverseMass != 0.0f)
    {
        Vect impulsiveTorque = (impulse).cross(relPos[1]);
        angVelocityChange[1] = impulsiveTorque * blocks[1]->inverseInertiaTensorWorld;
        velocityChange[1].set(0.0f, 0.0f, 0.0f);
		velocityChange[1] += impulse * -blocks[1]->inverseMass;

        // Apply changes to velocity and angular velocity
		blocks[1]->velocity += velocityChange[1];
		blocks[1]->angVelocity += angVelocityChange[1];
    }
};

void PhysicsContact::ChangePosition()
{
    // If not penetration, nothing to do
    if (penetration <= 0.0f) return;

    // Get total inverse mass
	float totalInvMass = blocks[0]->inverseMass + blocks[1]->inverseMass;

    // Do nothing if all particles have infinite mass
    if (totalInvMass <= 0.0f) return;

    // Find penetration resolution per unit of inverse mass
    Vect movePerInvMass = this->normal * (penetration / totalInvMass);

    // Find movement for each body
    Vect bodyMovement[2];
    bodyMovement[0] = movePerInvMass * blocks[0]->inverseMass;
    bodyMovement[1] = movePerInvMass * -1.0f * blocks[1]->inverseMass;

    // Update positions
    Vect positions[2];
	positions[0] = blocks[0]->position;
	positions[1] = blocks[1]->position;
    blocks[0]->position = (positions[0] + bodyMovement[0]);
    blocks[1]->position = (positions[1] + bodyMovement[1]);
};

// Calculate local orthonormal basis,
// With matrices to convert to and from
void PhysicsContact::CalculateBasis()
{
    Vect basisVectX;
    Vect basisVectY;

    // Check whether z-axis is closer to x or y axis
    if (abs(normal[0]) > abs(normal[1]))
    {
        // scaling factor to normalize results
        const float scale = 1.0f / sqrt(normal[2] * normal[2] + normal[0] * normal[0]);

        // Calculate new x axis
        basisVectX.set( normal[2] * scale, 0.0f, -normal[0] * scale, 0.0f);

        // Calculate new y axis
        basisVectY.set( normal[1] * basisVectX[0],
                        normal[2] * basisVectX[0] - normal[0] * basisVectX[2],
                        -normal[1] * basisVectX[0],
                        0.0f);
    }
    else
    {
        // scaling factor to normalize results
        const float scale = 1.0f / sqrt(normal[2] * normal[2] + normal[1] * normal[1]);

        // Calculate new x axis
        basisVectX.set( 0.0f, -normal[2] * scale, normal[1] * scale, 0.0f);

        // Calculate new y axis
        basisVectY.set( normal[1] * basisVectX[2] - normal[2] * basisVectX[1],
                        -normal[0] * basisVectX[2],
                        normal[0] * basisVectX[1],
                        0.0f );
    }

	Vect shouldBeNormal = basisVectX.cross(basisVectY);

    contactToWorld.set( basisVectX, basisVectY, normal, Vect(0.0, 0.0f, 0.0f, 1.0) );
	contactToWorld[11] = 0.0f;
    worldToContact = contactToWorld.getT();
};

// Calculate internal properties necessary for resolution
void PhysicsContact::CalculateData(const float timeIn)
{
	if (blocks[0] == 0 && blocks[1] == 0) return;

    // Simple check to see if only bodies[1] exists
    // Switch it to bodies[0] if so
    if (blocks[0] == 0 || blocks[0]->inverseMass == 0.0f)
    {
        this->normal *= -1.0f;
        Block* tmp = blocks[1];
        blocks[1] = blocks[0];
        blocks[0] = tmp;
    }

    // Calculate the contact basis
    this->CalculateBasis();

    // Calculate relative positions (of collision point with respect to each body)
	relPos[0] = this->contactPoint - blocks[0]->position;
    if (blocks[1] != 0)
    {
        relPos[1] = this->contactPoint - blocks[1]->position;
    }

    // Now calculate relative velocity of the 2 bodies, at the contact point
    this->contactVelocity = this->CalculateLocalVelocity(0, timeIn);
    if (blocks[1] != 0)
    {
        this->contactVelocity -= CalculateLocalVelocity(1, timeIn);
    }

    // Now calculate the desired change in velocity
    this->CalculateDesiredDeltaVelocity(timeIn);
};

// Calculate local velocity of a body at the contact point
Vect PhysicsContact::CalculateLocalVelocity(const int blockIndex, const float timeIn)
{
    // Make sure index is valid
    assert(blockIndex == 0 || blockIndex == 1);
    assert(blocks[blockIndex] != 0);

    Block* body = blocks[blockIndex];

    // Calculate the velocity at contact point
    Vect velocity = body->angVelocity.cross(relPos[blockIndex]);
    velocity += body->velocity;

    // Convert to contact coordinates
    velocity *= this->worldToContact;

    return velocity;
};

