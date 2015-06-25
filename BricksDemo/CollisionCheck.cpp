#include "CollisionCheck.h"
#include "Block.h"
#include "PhysicsContact.h"
#include <math.h>
#include <float.h>
#include <assert.h>

// Fill in data for a point to face collision
void fillDataPointFace(
	 Block& blockOne,
	 Block& blockTwo,
	const Vect& toCenter,
	PhysicsContact& pContact,
	unsigned int best,
	float penetration)
{
	// If this is called, we know vertex from two is in contact with one
	Matrix transOne = blockOne.transformMatrix;
	Matrix transTwo = blockTwo.transformMatrix;

	// We know which axis the collision is on (i.e. best),
	// but we need to work out which of the two faces on
	// this axis.
	Vect normal;
	if (best == 0)
	{
		normal = transOne.v0;
	}
	else if (best == 1)
	{
		normal = transOne.v1;
	}
	else
	{
		normal = transOne.v2;
	}
	if (normal.dot(toCenter) > 0.0f)
	{
		normal = normal * -1.0f;
	}

	// Work out which vertex of box two we're colliding with.
	// Using toCentre doesn't work!
	Vect vertex = blockTwo.scale * 0.5f;
	if (transTwo.v0.dot(normal) < 0) vertex[0] = -vertex[0];
	if (transTwo.v1.dot(normal) < 0) vertex[1] = -vertex[1];
	if (transTwo.v2.dot(normal) < 0) vertex[2] = -vertex[2];

	// Now fill in the contact data
	pContact.normal = normal;
	pContact.penetration = penetration;
	pContact.contactPoint = vertex * transTwo;
	pContact.blocks[0] = &blockOne;
	pContact.blocks[1] = &blockTwo;
};


#define TEST_AXIS(axis, index) \
	if ( !tryAxis(blockOne, blockTwo, (axis), diffCenter, (index), penetration, bestIndex)) return 0;

bool CheckColliding(Block& blockOne, Block& blockTwo, PhysicsContact& contact)
{
	// Calculate difference of centers
	Matrix transOne = blockOne.transformMatrix;;
	Matrix transTwo = blockTwo.transformMatrix;
	Vect diffCenter = transTwo.v3 - transOne.v3;

	// Initially assume there is no contact at all
	float penetration = FLT_MAX;
	unsigned bestIndex = 0xFFFFFF;

	// Now we check each axis, and return if it shows boxes are not colliding
	// Also keep track of smallest penetration
	Vect axis0(transOne.v0);
	TEST_AXIS(axis0, 0);
	Vect axis1(transOne.v1);
	TEST_AXIS(axis1, 1);
	Vect axis2(transOne.v2);
	TEST_AXIS(axis2, 2);

	Vect axis3(transTwo.v0);
	TEST_AXIS(axis3, 3);
	Vect axis4(transTwo.v1);
	TEST_AXIS(axis4, 4);
	Vect axis5(transTwo.v2);
	TEST_AXIS(axis5, 5);

	// Store the best axis-major, in case we run into almost
	// parallel edge collisions later
	unsigned bestSingleAxis = bestIndex;

	Vect axis6 = transOne.v0.cross(transTwo.v0);
	TEST_AXIS(axis6, 6);
	Vect axis7 = transOne.v0.cross(transTwo.v1);
	TEST_AXIS(axis7, 7);
	Vect axis8 = transOne.v0.cross(transTwo.v2);
	TEST_AXIS(axis8, 8);
	Vect axis9 = transOne.v1.cross(transTwo.v0);
	TEST_AXIS(axis9, 9);
	Vect axis10 = transOne.v1.cross(transTwo.v1);
	TEST_AXIS(axis10, 10);
	Vect axis11 = transOne.v1.cross(transTwo.v2);
	TEST_AXIS(axis11, 11);
	Vect axis12 = transOne.v2.cross(transTwo.v0);
	TEST_AXIS(axis12, 12);
	Vect axis13 = transOne.v2.cross(transTwo.v1);
	TEST_AXIS(axis13, 13);
	Vect axis14 = transOne.v2.cross(transTwo.v2);
	TEST_AXIS(axis14, 14);

	// Make sure we've got a result.
	if (bestIndex == 0xffffff) return 0;
	assert(bestIndex != 0xffffff);

	// We now know there's a collision, and we know which
	// of the axes gave the smallest penetration. We now
	// can deal with it in different ways depending on
	// the case.
	if (bestIndex < 3)
	{
		// We've got a vertex of box two on a face of box one.
		fillDataPointFace(blockOne, blockTwo, diffCenter, contact, bestIndex, penetration);
	}
	else if (bestIndex < 6)
	{
		// We've got a vertex of box one on a face of box two.
		// We use the same algorithm as above, but swap around
		// one and two (and therefore also the vector between their
		// centers).
		fillDataPointFace(blockTwo, blockOne, diffCenter * -1.0f, contact, bestIndex - 3, penetration);
	}
	else
	{
		// We've got an edge-edge contact. Find out which axes
		bestIndex -= 6;
		unsigned oneAxisIndex = bestIndex / 3;
		unsigned twoAxisIndex = bestIndex % 3;
		//Vect oneAxis = transOne.getAxisVector(oneAxisIndex);
		Vect oneAxis = transOne.v[oneAxisIndex];
		Vect twoAxis = transTwo.v[twoAxisIndex];
		Vect axis = oneAxis.cross(twoAxis);
		axis.norm();

		// The axis should point from box one to box two.
		if (axis.dot(diffCenter) > 0.0f) axis = axis * -1.0f;

		// We have the axes, but not the edges: each axis has 4 edges parallel
		// to it, we need to find which of the 4 for each object. We do
		// that by finding the point in the center of the edge. We know
		// its component in the direction of the box's collision axis is zero
		// (its a mid-point) and we determine which of the extremes in each
		// of the other axes is closest.
		Vect ptOnOneEdge = blockOne.scale * 0.5f;
		Vect ptOnTwoEdge = blockTwo.scale * 0.5f;
		for (unsigned i = 0; i < 3; i++)
		{
			if (i == oneAxisIndex) ptOnOneEdge[i] = 0.0f;
			else if (transOne.v[i].dot(axis) > 0.0f) ptOnOneEdge[i] = -ptOnOneEdge[i];

			if (i == twoAxisIndex) ptOnTwoEdge[i] = 0.0f;
			else if (transTwo.v[i].dot(axis) < 0.0f) ptOnTwoEdge[i] = -ptOnTwoEdge[i];
		}

		// Move them into world coordinates (they are already oriented
		// correctly, since they have been derived from the axes).
		ptOnOneEdge = ptOnOneEdge * transOne;
		ptOnTwoEdge = ptOnTwoEdge * transTwo;

		// So we have a point and a direction for the colliding edges.
		// We need to find out point of closest approach of the two
		// line-segments.
		Vect halfSizeOne = blockOne.scale * 0.5f;
		Vect halfSizeTwo = blockTwo.scale * 0.5f;

		Vect vertex = contactPoint(
			ptOnOneEdge, oneAxis, halfSizeOne[oneAxisIndex],
			ptOnTwoEdge, twoAxis, halfSizeTwo[twoAxisIndex],
			bestSingleAxis > 2
			);

		// We can fill the contact.
		contact.penetration = penetration;
		contact.normal = axis;
		contact.contactPoint = vertex;
		contact.blocks[0] = &blockOne;
		contact.blocks[1] = &blockTwo;
	}

	return 1;
};

