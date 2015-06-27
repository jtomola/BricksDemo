#include "Block.h"
#include "Demo.h"

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
		gravity(true),
		active(true)

{
};

void Block::Draw()
{
	if (!active) return;

	Matrix Trans;
	Trans.setTrans(position[0], position[1], position[2]);

	Matrix Rot;
	Rot.set(rotation);

	Matrix Scale;
	Scale.setScale(scale[0], scale[1], scale[2]);

	//Matrix ModelView(Scale);
	Matrix ModelView = Scale * Rot * Trans * Demo::GetCamera()->getViewMatrix();
	//Matrix ModelView = Scale * Trans * Demo::GetCamera()->getViewMatrix();

	Demo::SetModelView(ModelView);
	Demo::SetColorInfo(color);

	Demo::GetDeviceContext()->DrawIndexed(12 * 3, 0, 0);
};

void Block::Update(const float elapsedTime)
{
	if (!active) return;

	// Return if mass is infinite (ground)
	if (inverseMass <= 0.0f)
	{
		return;
	}
	
	if (!velocity.isZero()) this->gravity = true;

	// Update position
	this->position += (this->velocity * elapsedTime);

	// Update rotation
	if (!angVelocity.isZero())
	{
		Quat q;
		q.setRotXYZ(angVelocity[0] * elapsedTime, angVelocity[1] * elapsedTime, angVelocity[2] * elapsedTime);
		this->rotation = this->rotation * q;
	}

	// Update acceleration
	this->acceleration.set(0.0f, 0.0f, 0.0f);
	// Apply gravity
	if (this->gravity) this->acceleration += Vect (0.0f, -100.0f, 0.0f);
	this->acceleration += (force * inverseMass);

	// Update angular acceleration
	this->angAcceleration.set(0.0f, 0.0f, 0.0f);
	this->angAcceleration += torque * this->inverseInertiaTensorWorld;

	// Update velocity and angular velocity
	this->velocity += (acceleration * elapsedTime);
	this->angVelocity += (this->angAcceleration * elapsedTime);

	// Damp our velocities a bit
	this->velocity *= powf(0.85f, elapsedTime);
	this->angVelocity *= powf(0.5f, elapsedTime);

	// Calculate transform matrices and world inverse inertia tensor
	this->CalculateDerivedData();

	// Zero out our torques and forces
	this->force.set(0.0f, 0.0f, 0.0f);
	this->torque.set(0.0f, 0.0f, 0.0f);
};

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
		Matrix inertialTensor;
		float mass = 1.0f / inverseMass;
		inertialTensor.setScale((scale[1] * scale[1] + scale[2] * scale[2]) * mass / 12.0f,
			(scale[0] * scale[0] + scale[2] * scale[2]) * mass / 12.0f,
			(scale[0] * scale[0] + scale[1] * scale[1]) * mass / 12.0f);

		inverseInertiaTensor = inertialTensor.getInv();
	}


}

