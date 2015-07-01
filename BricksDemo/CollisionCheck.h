#include <math.h>
#include <float.h>
#include <assert.h>
#include "Block.h"

class PhysicsContact;

// Check if two blocks are colliding, fill contact data if so
bool CheckColliding(Block& blockOne, Block& blockTwo, PhysicsContact& contact);

// Fill contact data for a point face collision
void fillContactPointFaceCollision(
	Block& blockOne,
	Block& blockTwo,
	const Vect& toCenter,
	PhysicsContact& pContact,
	unsigned int best,
	float penetration);

// Transform an block to a length in a given axis
// Used for separating axis tests
static inline float transToAxis(Block& blockIn,
	const Vect& axisIn)
{
	Matrix transMat = blockIn.transformMatrix;

	Vect halfSize = blockIn.scale * 0.5f;
	return
		halfSize[0] * abs(axisIn.dot(transMat.v0)) +
		halfSize[1] * abs(axisIn.dot(transMat.v1)) +
		halfSize[2] * abs(axisIn.dot(transMat.v2));
};

// determine how much the objects penetrate along a given axis
static inline float penOnAxis(Block& blockOne,
	Block& blockTwo,
	const Vect& axis,
	const Vect& diffCenter)
{
	// Projections of each box
	const float oneProjection = transToAxis(blockOne, axis);
	const float twoProjection = transToAxis(blockTwo, axis);

	// Distance between block centers on this axis
	const float dist = abs(diffCenter.dot(axis));

	// Calculate and return the overlap
	return oneProjection + twoProjection - dist;
};

// Test whether blocks are penetrating along a given axis
// Returns bool and updates the smallest penetration if necessary
static inline bool testAxis(
	Block& blockOne,
	Block& blockTwo,
	Vect& axis,
	const Vect& toCenter,
	unsigned index,

	float& smallestPenetration, // updated by this function
	unsigned& smallestCase
	)
{
	// No need to check if lines are parallel
	if (axis.magSqr() < 0.0001f) return true;
	// Normalize the axis
	axis.norm();

	// Calculate penetration on this axis
	const float pen = penOnAxis(blockOne, blockTwo, axis, toCenter);

	// Update smallest penetration if necessary
	if (pen < 0.0f) return false;
	if (pen < smallestPenetration) {
		smallestPenetration = pen;
		smallestCase = index;
	}
	return true;
};

// Calculate the contact point for an edge to edge collision
static inline Vect contactPointEdgeEdge(
	const Vect& ptOnEdgeBlockOne,
	const Vect& oneAxis,
	float sizeOne,
	const Vect& ptOnEdgeBlockTwo,
	const Vect& twoAxis,
	float sizeTwo,
	bool useOne)
{
	// All our variables
	Vect diffPos, cOne, cTwo;
	float diffPosAxisOne, diffPosAxisTwo, axisDotProduct;
	float denom, nearestPointOne, nearestPointTwo;

	axisDotProduct = twoAxis.dot(oneAxis);

	diffPos = ptOnEdgeBlockOne - ptOnEdgeBlockTwo;
	diffPosAxisOne = oneAxis.dot(diffPos);
	diffPosAxisTwo = twoAxis.dot(diffPos);

	denom = 1.0f - axisDotProduct * axisDotProduct;

	// Check for parallel lines
	if (abs(denom) < 0.0001f) {
		return useOne ? ptOnEdgeBlockOne : ptOnEdgeBlockTwo;
	}

	nearestPointOne = (axisDotProduct * diffPosAxisTwo - diffPosAxisOne) / denom;
	nearestPointTwo = (diffPosAxisTwo - axisDotProduct * diffPosAxisOne) / denom;

	// If nearest point is not in the block, just return one of the midpoints
	if (nearestPointOne > sizeOne ||
		nearestPointOne < -sizeOne ||
		nearestPointTwo > sizeTwo ||
		nearestPointTwo < -sizeTwo)
	{
		return useOne ? ptOnEdgeBlockOne : ptOnEdgeBlockTwo;
	}
	else
	{
		cOne = ptOnEdgeBlockOne + oneAxis * nearestPointOne;
		cTwo = ptOnEdgeBlockTwo + twoAxis * nearestPointTwo;

		return cOne * 0.5 + cTwo * 0.5;
	}
}
