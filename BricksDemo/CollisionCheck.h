#include <math.h>
#include <float.h>
#include <assert.h>
#include "Block.h"

class PhysicsContact;

bool CheckColliding(Block& blockOne, Block& blockTwo, PhysicsContact& contact);
void fillDataPointFace(
	Block& blockOne,
	Block& blockTwo,
	const Vect& toCenter,
	PhysicsContact& pContact,
	unsigned int best,
	float penetration);

// Transform an AABB to a length in a given axis
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
	//const Matrix& transOne,
	//const Matrix& transTwo,
	const Vect& axis,
	const Vect& diffCenter)
{
	// Projections of each box
	const float oneProjection = transToAxis(blockOne, axis);
	const float twoProjection = transToAxis(blockTwo, axis);

	// Distance between centers on this axis
	const float dist = abs(diffCenter.dot(axis));

	// Calculate and return the overlap
	return oneProjection + twoProjection - dist;
};

static inline bool tryAxis(
	Block& blockOne,
	Block& blockTwo,
	Vect& axis,
	const Vect& toCenter,
	unsigned index,

	// These values may be updated
	float& smallestPenetration,
	unsigned& smallestCase
	)
{
	// Make sure we have a normalized axis, and don't check almost parallel axes
	if (axis.magSqr() < 0.0001) return true;
	axis.norm();

	// Calculate penetration on this axis
	const float pen = penOnAxis(blockOne, blockTwo, axis, toCenter);

	// Update smallest penetration if necessary
	if (pen < 0.0f) return false;
	if (pen <= smallestPenetration + 0.01f) {
		smallestPenetration = pen;
		smallestCase = index;
	}
	return true;
};

static inline Vect contactPoint(
	const Vect& pOne,
	const Vect& dOne,
	float oneSize,
	const Vect& pTwo,
	const Vect& dTwo,
	float twoSize,

	// If this is true, and the contact point is outside
	// the edge (in the case of an edge-face contact) then
	// we use one's midpoint, otherwise we use two's.
	bool useOne)
{
	Vect toSt, cOne, cTwo;
	float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
	float denom, mua, mub;

	smOne = dOne.magSqr();
	smTwo = dTwo.magSqr();
	dpOneTwo = dTwo.dot(dOne);

	toSt = pOne - pTwo;
	dpStaOne = dOne.dot(toSt);
	dpStaTwo = dTwo.dot(toSt);

	denom = smOne * smTwo - dpOneTwo * dpOneTwo;

	// Zero denominator indicates parallel lines
	if (abs(denom) < 0.0001f) {
		return useOne ? pOne : pTwo;
	}

	mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
	mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

	// If either of the edges has the nearest point out
	// of bounds, then the edges aren't crossed, we have
	// an edge-face contact. Our point is on the edge, which
	// we know from the useOne parameter.
	if (mua > oneSize ||
		mua < -oneSize ||
		mub > twoSize ||
		mub < -twoSize)
	{
		return useOne ? pOne : pTwo;
	}
	else
	{
		cOne = pOne + dOne * mua;
		cTwo = pTwo + dTwo * mub;

		return cOne * 0.5 + cTwo * 0.5;
	}
}
