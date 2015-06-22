#include <math.h>
#include <assert.h>
#include "Quat.h"

//Default constructor
Quat::Quat()
	: qx(0.0f), qy(0.0f), qz(0.0f), qw(1.0f)
{
};

// Copy constructor
Quat::Quat(const Quat& quatIn)
	: qVect(quatIn.qVect)
{
};

Quat::Quat(const float qxIn, const float qyIn, const float qzIn, const float qwIn)
	: qVect( Vect(qxIn, qyIn, qzIn, qwIn) )
{
};

Quat& Quat::operator= (const Quat& rhs)
{
    if (this != &rhs)
	{
        this->qx = rhs.qx;
        this->qy = rhs.qy;
        this->qz = rhs.qz;
        this->qw = rhs.qw;
	}

    return *this;
};

void Quat::set(const float qxIn, const float qyIn, const float qzIn, const float qwIn)
{
    this->qx = qxIn;
    this->qy = qyIn;
    this->qz = qzIn;
    this->qw = qwIn;
};

void Quat::set(const Quat& quatIn)
{
    this->qVect = quatIn.qVect;
};

void Quat::setRotXYZ(const float rotxIn, const float rotyIn, const float rotzIn)
{
	Quat RotX; 
	Quat RotY;
	Quat RotZ;

	RotX.setAxisAngle(Vect(1.0f, 0.0f, 0.0f), rotxIn);
	RotY.setAxisAngle(Vect(0.0f, 1.0f, 0.0f), rotxIn);
	RotZ.setAxisAngle(Vect(0.0f, 0.0f, 1.0f), rotxIn);

    this->set( RotX * RotY * RotZ);
};

void Quat::setAxisAngle(const Vect& axisIn, const float angleIn)
{
	this->qVect = axisIn.getNorm() * sinf(angleIn * 0.5f);
	this->qw = cosf(angleIn * 0.5f);
}

Quat Quat::operator*(const Quat& quatIn) const
{
    Quat returnQuat;
    returnQuat.qVect = quatIn.qVect.cross(this->qVect) + quatIn.qw * this->qVect + this->qw * quatIn.qVect;
    returnQuat.qw = this->qw * quatIn.qw - this->qVect.dot(quatIn.qVect);

    return returnQuat;
};


const float Quat::operator[](const int indexIn) const
{
	assert(indexIn >= 0 && indexIn <= 3);
    return this->q[indexIn];
}

float& Quat::operator[](const int indexIn)
{
	assert(indexIn >= 0 && indexIn <= 3);
    return this->q[indexIn];
}
