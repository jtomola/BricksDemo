#include <math.h>
#include "Vect.h"
#include "Matrix.h"
#include <assert.h>

// do magic
Vect::Vect()
	: vx(0.0f), vy(0.0f), vz(0.0f), vw(1.0f)
{
}

Vect::Vect(const float inX, const float inY, const float inZ, const float inW)
	: vx(inX), vy(inY), vz(inZ), vw(inW)
{
}

Vect::Vect(const Vect& vectIn)
	: vx(vectIn.vx), vy(vectIn.vy), vz(vectIn.vz), vw(vectIn.vw)
{
}

Vect& Vect::operator=(const Vect& rhs)
{
    if (this != &rhs)
	{
        this->vx = rhs.vx;
        this->vy = rhs.vy;
        this->vz = rhs.vz;
        this->vw = rhs.vw;
	}
    return *this;
}

void Vect::set(const float inX, const float inY, const float inZ, const float inW)
{
    this->vx = inX;
    this->vy = inY;
    this->vz = inZ;
    this->vw = inW;
}

void Vect::set(const Vect& rhs)
{
    if (this != &rhs)
	{
        this->vx = rhs.vx;
        this->vy = rhs.vy;
        this->vz = rhs.vz;
        this->vw = rhs.vw;
	}
}

float& Vect::operator[](const unsigned int indexIn)
{
	assert(indexIn >= 0 && indexIn <= 3);

	return this->v[indexIn];
}

const float Vect::operator[](const unsigned int indexIn) const
{
	assert(indexIn >= 0 && indexIn <= 3);

	return this->v[indexIn];
}

Vect Vect::operator+(const Vect& vectIn) const
{
    return Vect(this->vx + vectIn.vx, this->vy + vectIn.vy, this->vz + vectIn.vz);
}

Vect& Vect::operator+=(const Vect& vectIn)
{
    this->vx += vectIn.vx;
    this->vy += vectIn.vy;
    this->vz += vectIn.vz;
    this->vw = 1.0f;
    return *this;
}

Vect Vect::operator-(const Vect& vectIn) const
{
    return Vect(this->vx - vectIn.vx, this->vy - vectIn.vy, this->vz - vectIn.vz);
}

Vect& Vect::operator-=(const Vect& vectIn)
{
    this->vx -= vectIn.vx;
    this->vy -= vectIn.vy;
    this->vz -= vectIn.vz;
    this->vw = 1.0f;
    return *this;
}

Vect operator*(const float s, const Vect& vectIn)
{
    return Vect(s * vectIn.vx, s * vectIn.vy, s * vectIn.vz);
}

Vect Vect::operator*(const float s) const
{
    return Vect(s * this->vx, s * this->vy, s * this->vz);
}

Vect& Vect::operator*=(const float s)
{
    this->vx *= s;
    this->vy *= s;
    this->vz *= s;
    this->vw = 1.0f;
    return *this;
}

float Vect::dot(const Vect& vectIn) const
{
    return this->vx * vectIn.vx + this->vy * vectIn.vy + this->vz * vectIn.vz;
}

Vect Vect::cross(const Vect& vectIn) const
{
    return Vect(this->vy * vectIn.vz - vectIn.vy * this->vz,
        -(this->vx * vectIn.vz - vectIn.vx * this->vz),
        this->vx * vectIn.vy - vectIn.vx * this->vy);
}

void Vect::norm()
{
    float f = 1 / sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);

    this->vx *= f;
    this->vy *= f;
    this->vz *= f;
    this->vw = 1.0f;
}

Vect Vect::getNorm() const
{
    Vect returnVect;

    float f = 1 / sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);

    returnVect.vx = this->vx * f;
    returnVect.vy = this->vy * f;
    returnVect.vz = this->vz * f;

    return returnVect;
}

float Vect::mag() const
{
    return sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);
}

float Vect::magSqr() const
{
    return this->vx * this->vx + this->vy * this->vy + this->vz * this->vz;
}

bool Vect::isZero(const float tolerance /*= 0.0001f*/) const
{
	bool retBool = true;

	retBool &= ((this->v[0] >= -tolerance) && (this->v[0] <= tolerance));
	retBool &= ((this->v[1] >= -tolerance) && (this->v[1] <= tolerance));
	retBool &= ((this->v[2] >= -tolerance) && (this->v[2] <= tolerance));

	return retBool;
}

bool Vect::isEqual(const Vect& vectIn, const float tolerance /*= 0.0001f*/) const
{
	bool retBool = true;

	float tmp = 0.0f;

	tmp = this->v[0] - vectIn.v[0];
	retBool &= ((tmp >= -tolerance) && (tmp <= tolerance));
	tmp = this->v[1] - vectIn.v[1];
	retBool &= ((tmp >= -tolerance) && (tmp <= tolerance));
	tmp = this->v[2] - vectIn.v[2];
	retBool &= ((tmp >= -tolerance) && (tmp <= tolerance));

	return retBool;
};

Vect Vect::operator*(const Matrix& matrixIn) const
{
    Vect returnVect;

    returnVect.vx = this->vx * matrixIn[0] + this->vy * matrixIn[4] + this->vz * matrixIn[8] + this->vw * matrixIn[12];
    returnVect.vy = this->vx * matrixIn[1] + this->vy * matrixIn[5] + this->vz * matrixIn[9] + this->vw * matrixIn[13];
    returnVect.vz = this->vx * matrixIn[2] + this->vy * matrixIn[6] + this->vz * matrixIn[10] + this->vw * matrixIn[14];
    returnVect.vw = this->vx * matrixIn[3] + this->vy * matrixIn[7] + this->vz * matrixIn[11] + this->vw * matrixIn[15];

    return returnVect;
}

Vect& Vect::operator*=(const Matrix& matrixIn)
{
    Vect returnVect;

    returnVect.vx = this->vx * matrixIn[0] + this->vy * matrixIn[4] + this->vz * matrixIn[8] + this->vw * matrixIn[12];
    returnVect.vy = this->vx * matrixIn[1] + this->vy * matrixIn[5] + this->vz * matrixIn[9] + this->vw * matrixIn[13];
    returnVect.vz = this->vx * matrixIn[2] + this->vy * matrixIn[6] + this->vz * matrixIn[10] + this->vw * matrixIn[14];
    returnVect.vw = this->vx * matrixIn[3] + this->vy * matrixIn[7] + this->vz * matrixIn[11] + this->vw * matrixIn[15];

    *this = returnVect;

    return *this;
}