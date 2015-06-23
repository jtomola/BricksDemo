#include "Brick.h"
#include "Demo.h"

Brick::Brick()
	:	transformMatrix(),
		inverseInertiaTensor(),
		inverseInertiaTensorWorld(),
		position(),
		velocity(),
		acceleration(),
		rotation(),
		angVelocity(),
		angAcceleration(),
		scale(20.0f, 20.0f, 20.0f),
		color(1.0f, 0.0f, 0.0f, 1.0f),
		inverseMass(0.0f)

{
};

void Brick::Draw()
{
	Matrix Trans;
	Trans.setTrans(position[0], position[1], position[2]);

	Matrix Scale;
	Scale.setScale(scale[0], scale[1], scale[2]);

	//Matrix ModelView(Scale);
	Matrix ModelView = Scale * Trans * Demo::GetCamera()->getViewMatrix();

	Demo::SetModelView(ModelView);
	Demo::SetColorInfo(color);

	Demo::GetDeviceContext()->DrawIndexed(12 * 3, 0, 0);
};

void Brick::Update(const float elapsedTime)
{
	// Return if mass is infinite (ground)
	if (inverseMass <= 0.0f)
	{
		return;
	}

	// Update position
	this->position += (this->velocity * elapsedTime);

	// Update rotation
	if (!angVelocity.isZero())
	{
		Quat q;
		q.setRotXYZ(angVelocity[0] * elapsedTime, angVelocity[1] * elapsedTime, angVelocity[2] * elapsedTime);
		this->rotation = this->rotation * q;
	}

	// Add gravity
	Vect usedAcceleration = this->acceleration + Vect(0.0f, -100.0f, 0.0f);

	// Update velocity and angular velocity
	this->velocity += (usedAcceleration * elapsedTime);
	this->angVelocity += (this->angAcceleration * elapsedTime);

	// Damp our velocities a bit
	this->velocity *= powf(0.95f, elapsedTime);
	this->angVelocity *= powf(0.95f, elapsedTime);

	// Calculate transform matrices and world inverse inertia tensor
	this->CalculateDerivedData();

	this->acceleration.set(Vect(0.0f, 0.0f, 0.0f));
	this->angAcceleration.set(Vect(0.0f, 0.0f, 0.0f));
};

void Brick::CalculateDerivedData()
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
