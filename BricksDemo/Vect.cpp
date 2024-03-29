#include <math.h>
#include "Vect.h"
#include "Matrix.h"
#include <assert.h>

// Default constructor
Vect::Vect()
	: vx(0.0f), vy(0.0f), vz(0.0f), vw(1.0f)
{
}

// Specialized constructor
Vect::Vect(const float inX, const float inY, const float inZ, const float inW)
	: vx(inX), vy(inY), vz(inZ), vw(inW)
{
}

// Copy constructor
Vect::Vect(const Vect& vectIn)
	: vx(vectIn.vx), vy(vectIn.vy), vz(vectIn.vz), vw(vectIn.vw)
{
}

// Assignment operator
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

// Set function for 4 floats
void Vect::set(const float inX, const float inY, const float inZ, const float inW)
{
    this->vx = inX;
    this->vy = inY;
    this->vz = inZ;
    this->vw = inW;
}

// Set function for another Vect
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

// Indexing for modifying
float& Vect::operator[](const unsigned int indexIn)
{
	assert(indexIn >= 0 && indexIn <= 3);

	return this->v[indexIn];
}

// Constant indexing
const float Vect::operator[](const unsigned int indexIn) const
{
	assert(indexIn >= 0 && indexIn <= 3);

	return this->v[indexIn];
}

// Addition
Vect Vect::operator+(const Vect& vectIn) const
{
    return Vect(this->vx + vectIn.vx, this->vy + vectIn.vy, this->vz + vectIn.vz);
}

// Addition (+=)
Vect& Vect::operator+=(const Vect& vectIn)
{
    this->vx += vectIn.vx;
    this->vy += vectIn.vy;
    this->vz += vectIn.vz;
    this->vw = 1.0f;
    return *this;
}

// Subtraction
Vect Vect::operator-(const Vect& vectIn) const
{
    return Vect(this->vx - vectIn.vx, this->vy - vectIn.vy, this->vz - vectIn.vz);
}

// Subtraction (-=)
Vect& Vect::operator-=(const Vect& vectIn)
{
    this->vx -= vectIn.vx;
    this->vy -= vectIn.vy;
    this->vz -= vectIn.vz;
    this->vw = 1.0f;
    return *this;
}

// Multiplication by float (friend function)
Vect operator*(const float s, const Vect& vectIn)
{
    return Vect(s * vectIn.vx, s * vectIn.vy, s * vectIn.vz);
}

// Multiplication by float
Vect Vect::operator*(const float s) const
{
    return Vect(s * this->vx, s * this->vy, s * this->vz);
}

// Multiplication by float (*=)
Vect& Vect::operator*=(const float s)
{
    this->vx *= s;
    this->vy *= s;
    this->vz *= s;
    this->vw = 1.0f;
    return *this;
}

// Dot product
float Vect::dot(const Vect& vectIn) const
{
    return this->vx * vectIn.vx + this->vy * vectIn.vy + this->vz * vectIn.vz;
}

// Cross product
Vect Vect::cross(const Vect& vectIn) const
{
    return Vect(this->vy * vectIn.vz - vectIn.vy * this->vz,
        -(this->vx * vectIn.vz - vectIn.vx * this->vz),
        this->vx * vectIn.vy - vectIn.vx * this->vy);
}

// Normalize (modifies the vector)
void Vect::norm()
{
    float f = 1 / sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);

    this->vx *= f;
    this->vy *= f;
    this->vz *= f;
    this->vw = 1.0f;
}

// Returns normalized vector (this object not modified)
Vect Vect::getNorm() const
{
    Vect returnVect;

    float f = 1 / sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);

    returnVect.vx = this->vx * f;
    returnVect.vy = this->vy * f;
    returnVect.vz = this->vz * f;

    return returnVect;
}

// Magnitude of a vector
float Vect::mag() const
{
    return sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);
}

// Magnitude of the vector squared (avoids square root)
float Vect::magSqr() const
{
    return this->vx * this->vx + this->vy * this->vy + this->vz * this->vz;
}

// Check if vector is zero vector (within tolerance)
bool Vect::isZero(const float tolerance /*= 0.0001f*/) const
{
	bool retBool = true;

	retBool &= ((this->v[0] >= -tolerance) && (this->v[0] <= tolerance));
	retBool &= ((this->v[1] >= -tolerance) && (this->v[1] <= tolerance));
	retBool &= ((this->v[2] >= -tolerance) && (this->v[2] <= tolerance));

	return retBool;
}

// Check if 2 vectors are equal (within tolerance)
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

// Multiplication by matrix
Vect Vect::operator*(const Matrix& matrixIn) const
{
    Vect returnVect;

    returnVect.vx = this->vx * matrixIn[0] + this->vy * matrixIn[4] + this->vz * matrixIn[8] + this->vw * matrixIn[12];
    returnVect.vy = this->vx * matrixIn[1] + this->vy * matrixIn[5] + this->vz * matrixIn[9] + this->vw * matrixIn[13];
    returnVect.vz = this->vx * matrixIn[2] + this->vy * matrixIn[6] + this->vz * matrixIn[10] + this->vw * matrixIn[14];
    returnVect.vw = this->vx * matrixIn[3] + this->vy * matrixIn[7] + this->vz * matrixIn[11] + this->vw * matrixIn[15];

    return returnVect;
}

// Multiplication by matrix (*=)
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