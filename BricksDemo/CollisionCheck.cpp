#include "CollisionCheck.h"
#include "Block.h"
#include "PhysicsContact.h"
#include <math.h>
#include <float.h>
#include <assert.h>

// Fill in data for a point to face collision
void fillContactPointFaceCollision(
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

	// We know the axis of the collision
	// Could be either of 2 faces
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


// Macro to test a given axis
// Updates the smallest penetration if necessary
// Returns if this axis shows we're not colliding
#define TEST_AXIS(axis, index) \
	if ( !testAxis(blockOne, blockTwo, (axis), diffCenter, (index), penetration, bestIndex)) return 0;

bool CheckColliding(Block& blockOne, Block& blockTwo, PhysicsContact& contact)
{
	if (!blockOne.active || !blockTwo.active) return false;

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

	unsigned bestSingleAxis = bestIndex;

	Vect axis6 = transOne.v0.cross(transTwo.v0);
	if (!axis6.isZero())
	{
		TEST_AXIS(axis6, 6);
	}
	Vect axis7 = transOne.v0.cross(transTwo.v1);
	if (!axis7.isZero())
	{
		TEST_AXIS(axis7, 7);
	}
	Vect axis8 = transOne.v0.cross(transTwo.v2);
	if (!axis8.isZero())
	{
		TEST_AXIS(axis8, 8);
	}
	Vect axis9 = transOne.v1.cross(transTwo.v0);
	if (!axis9.isZero())
	{
		TEST_AXIS(axis9, 9);
	}
	Vect axis10 = transOne.v1.cross(transTwo.v1);
	if (!axis10.isZero())
	{
		TEST_AXIS(axis10, 10);
	}
	Vect axis11 = transOne.v1.cross(transTwo.v2);
	if (!axis11.isZero())
	{
		TEST_AXIS(axis11, 11);
	}
	Vect axis12 = transOne.v2.cross(transTwo.v0);
	if (!axis12.isZero())
	{
		TEST_AXIS(axis12, 12);
	}
	Vect axis13 = transOne.v2.cross(transTwo.v1);
	if (!axis13.isZero())
	{
		TEST_AXIS(axis13, 13);
	}
	Vect axis14 = transOne.v2.cross(transTwo.v2);
	if (!axis14.isZero())
	{
		TEST_AXIS(axis14, 14);
	}

	// Make sure nothing went wrong
	if (bestIndex == 0xffffff) return 0;
	assert(bestIndex != 0xffffff);

	// If we get here we know there was a collision
	// Depending on which axis, we need to deal with it
	if (bestIndex < 3)
	{
		// Vertex of box two is colliding with face of box one. Fill our contact data
		fillContactPointFaceCollision(blockOne, blockTwo, diffCenter, contact, bestIndex, penetration);

		// This is a bit of a hack to improve performance
		// Need to check if any axes are aligned. Might need to adjust contact point to center it on a face
		// Previous function always returns a corner
		Vect bestAxis = transOne.v[bestIndex];

		// See if any of other block's axes align with this one
		bool matchingAxis = false;
		for (int k = 0; k < 3; k++)
		{
			if (bestAxis.isEqual(transTwo.v[k], 0.001f) ||
				bestAxis.isEqual(transTwo.v[k] * -1.0f, 0.001f))
			{
				matchingAxis = true;
			}
		}

		if (matchingAxis)
		{
			// See how many of block 1's corners are inside block 2
			Vect pointSum(0.0f, 0.0f, 0.0f);
			int numPointsInside = 0;

			Vect corner;
			corner = blockOne.GetCorner(MIN, MIN, MIN);
			if (blockTwo.PointInsideBlock(corner)) { pointSum += corner; numPointsInside++; }
			corner = blockOne.GetCorner(MIN, MIN, MAX);
			if (blockTwo.PointInsideBlock(corner)) { pointSum += corner; numPointsInside++; }
			corner = blockOne.GetCorner(MIN, MAX, MIN);
			if (blockTwo.PointInsideBlock(corner)) { pointSum += corner; numPointsInside++; }
			corner = blockOne.GetCorner(MIN, MAX, MAX);
			if (blockTwo.PointInsideBlock(corner)) { pointSum += corner; numPointsInside++; }
			corner = blockOne.GetCorner(MAX, MIN, MIN);
			if (blockTwo.PointInsideBlock(corner)) { pointSum += corner; numPointsInside++; }
			corner = blockOne.GetCorner(MAX, MIN, MAX);
			if (blockTwo.PointInsideBlock(corner)) { pointSum += corner; numPointsInside++; }
			corner = blockOne.GetCorner(MAX, MAX, MIN);
			if (blockTwo.PointInsideBlock(corner)) { pointSum += corner; numPointsInside++; }
			corner = blockOne.GetCorner(MAX, MAX, MAX);
			if (blockTwo.PointInsideBlock(corner)) { pointSum += corner; numPointsInside++; }

			// If 4 corners are penetrating, set the contact point to the center of the face
			if (numPointsInside == 4)
			{
				contact.contactPoint = pointSum * (1.0f / float(numPointsInside));
			}
		}
	}
	else if (bestIndex < 6)
	{
		// Vertex of box one is colliding with face of box two. Fill our contact data
		fillContactPointFaceCollision(blockTwo, blockOne, diffCenter * -1.0f, contact, bestIndex - 3, penetration);
	}
	else
	{
		// If we're here, it's an edge edge contact
		bestIndex -= 6;
		unsigned oneAxisIndex = bestIndex / 3;
		unsigned twoAxisIndex = bestIndex % 3;
		Vect oneAxis = transOne.v[oneAxisIndex];
		Vect twoAxis = transTwo.v[twoAxisIndex];
		Vect axis = oneAxis.cross(twoAxis);
		axis.norm();

		// If not pointing from box one to box two, correct it
		if (axis.dot(diffCenter) > 0.0f) axis = axis * -1.0f;

		// We know the axis, but need to figure out which edges are colliding
		// Find center points of the two edges
		Vect ptOnEdgeBlockOne = blockOne.scale * 0.5f;
		Vect ptOnEdgeBlockTwo = blockTwo.scale * 0.5f;
		for (unsigned int i = 0; i < 3; i++)
		{
			if (i == oneAxisIndex) ptOnEdgeBlockOne[i] = 0.0f;
			else if (transOne.v[i].dot(axis) > 0.0f) ptOnEdgeBlockOne[i] = -ptOnEdgeBlockOne[i];

			if (i == twoAxisIndex) ptOnEdgeBlockTwo[i] = 0.0f;
			else if (transTwo.v[i].dot(axis) < 0.0f) ptOnEdgeBlockTwo[i] = -ptOnEdgeBlockTwo[i];
		}

		// Convert these midpoints into world coordinates
		ptOnEdgeBlockOne = ptOnEdgeBlockOne * transOne;
		ptOnEdgeBlockTwo = ptOnEdgeBlockTwo * transTwo;

		Vect halfSizeOne = blockOne.scale * 0.5f;
		Vect halfSizeTwo = blockTwo.scale * 0.5f;

		// calculate the contact point
		Vect vertex = contactPointEdgeEdge(
			ptOnEdgeBlockOne, oneAxis, halfSizeOne[oneAxisIndex],
			ptOnEdgeBlockTwo, twoAxis, halfSizeTwo[twoAxisIndex],
			bestSingleAxis > 2
			);

		contact.penetration = penetration;
		contact.normal = axis;
		contact.contactPoint = vertex;
		contact.blocks[0] = &blockOne;
		contact.blocks[1] = &blockTwo;
	}

	if (contact.penetration > 0.0f)
	{
		return 1;
	}
	else
	{
		contact.Reset();
		return 0;
	}
};

