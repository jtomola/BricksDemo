#include "Block.h"
#include "Demo.h"

// Default constructor
Block::Block()
	:	transformMatrix(),
		inverseInertiaTensor(),
		inverseInertiaTensorWorld(),
		position(),
		velocity(),
		acceleration(),
		rotation(),
		angVelocity(),
		angAcceleration(),
		force(),
		torque(),
		scale(20.0f, 20.0f, 20.0f),
		color(1.0f, 0.0f, 0.0f, 1.0f),
		inverseMass(0.0f),
		gravityNow(false),
		gravityEver(true),
		active(true)

{
};

// Draw the block
void Block::Draw()
{
	if (!active) return;

	// Combine translation, rotation, and scale
	Matrix Trans;
	Trans.setTrans(position[0], position[1], position[2]);

	Matrix Rot;
	Rot.set(rotation);

	Matrix Scale;
	Scale.setScale(scale[0], scale[1], scale[2]);

	// Also multiply by camera's view matrix to get ModelView matrix
	Matrix ModelView = Scale * Rot * Trans * Demo::GetCamera()->getViewMatrix();

	// Pass the necessary info to demo class, which sends it to shader
	Demo::SetModelView(ModelView);
	Demo::SetColorInfo(color);

	// Actual draw call
	Demo::GetDeviceContext()->DrawIndexed(12 * 3, 0, 0);
};

	// Update the physics of the block
void Block::Update(const float elapsedTime)
{
	if (!active) return;

	// Return if mass is infinite (ground)
	if (inverseMass <= 0.0f)
	{
		return;
	}
	
	// Leave gravity off in the beginning until blocks start moving
	if (!velocity.isZero() && gravityEver) this->gravityNow = true;

	// Update position using velocity
	this->position += (this->velocity * elapsedTime);

	// Update rotation using angular velocity
	if (!angVelocity.isZero())
	{
		Quat q;
		q.setRotXYZ(angVelocity[0] * elapsedTime, angVelocity[1] * elapsedTime, angVelocity[2] * elapsedTime);
		this->rotation = this->rotation * q;
	}

	// Update acceleration
	this->acceleration.set(0.0f, 0.0f, 0.0f);
	// Apply gravity
	if (this->gravityNow) this->acceleration += Vect (0.0f, -100.0f, 0.0f);
	this->acceleration += (force * inverseMass);

	// Update angular acceleration
	this->angAcceleration.set(0.0f, 0.0f, 0.0f);
	this->angAcceleration += torque * this->inverseInertiaTensorWorld;

	// Update velocity and angular velocity
	this->velocity += (acceleration * elapsedTime);
	this->angVelocity += (this->angAcceleration * elapsedTime);

	// Damp our velocities a bit
	this->velocity *= powf(0.5f, elapsedTime);
	this->angVelocity *= powf(0.5f, elapsedTime);

	// Calculate transform matrices and world inverse inertia tensor
	this->CalculateDerivedData();

	// Zero out our torques and forces
	this->force.set(0.0f, 0.0f, 0.0f);
	this->torque.set(0.0f, 0.0f, 0.0f);
};

// Calculate the necessary values for collisions each frame
void Block::CalculateDerivedData()
{
	// Set our transform matrix
	Matrix Trans;
	Trans.setTrans(this->position[0], this->position[1], this->position[2]);
	Matrix Rot;
	Rot.set(this->rotation);
	this->transformMatrix = Rot * Trans;

	// Transform our inertial tensor into world space
	this->inverseInertiaTensorWorld = Rot * this->inverseInertiaTensor* Rot.getT();
};

// Calculate the inverse inertial tensor based on mass and box size
void Block::CalcInertiaTensor()
{
	// Mass should be set already
	if (inverseMass == 0.0f)
	{
		this->inverseInertiaTensor = Matrix();
		this->inverseInertiaTensor[15] = 1.0f;
	}
	else
	{
		// Calculate inertial tensor matrix
		Matrix inertialTensor;
		float mass = 1.0f / inverseMass;
		inertialTensor.setScale((scale[1] * scale[1] + scale[2] * scale[2]) * mass / 12.0f,
			(scale[0] * scale[0] + scale[2] * scale[2]) * mass / 12.0f,
			(scale[0] * scale[0] + scale[1] * scale[1]) * mass / 12.0f);

		// Then take inverse
		inverseInertiaTensor = inertialTensor.getInv();
	}
};

// Test whether a point in world space is inside the block
bool Block::PointInsideBlock(const Vect& pointIn)
{
	// Need to convert this point into the coordinate space of our block

	// This is orthonormal, can take transpose to get inverse
	Matrix WorldToLocal;
	WorldToLocal.set(this->rotation);
	WorldToLocal.T();

	// Now apply opposite of translation
	WorldToLocal[12] = -position[0];
	WorldToLocal[13] = -position[1];
	WorldToLocal[14] = -position[2];

	// Multiply pointIn by this matrix to convert to local space
	Vect localPoint = pointIn * WorldToLocal;

	Vect halfsize = this->scale * 0.5f;

	// Check whether point is within the block in each axis
	bool retBool = true;
	retBool &= (localPoint[0] >= -halfsize[0] && localPoint[0] <= halfsize[0]);
	retBool &= (localPoint[1] >= -halfsize[1] && localPoint[1] <= halfsize[1]);
	retBool &= (localPoint[2] >= -halfsize[2] && localPoint[2] <= halfsize[2]);

	return retBool;
};

// Get a corner of the block in world space
Vect Block::GetCorner(const MinMax x, const MinMax y, const MinMax z)
{
	Vect corner = this->position;
	Vect xDiff = this->scale[0] * 0.5f * this->transformMatrix.v[0];
	if (x == MIN) xDiff *= -1.0f;
	Vect yDiff = this->scale[1] * 0.5f * this->transformMatrix.v[1];
	if (y == MIN) yDiff *= -1.0f;
	Vect zDiff = this->scale[2] * 0.5f * this->transformMatrix.v[2];
	if (z == MIN) zDiff *= -1.0f;

	corner += (xDiff + yDiff + zDiff);

	return corner;
};
