#include "Matrix.h"
#include <math.h>
#include <string.h>
#include <assert.h>
#include "Quat.h"

// Default Constructor
Matrix::Matrix()
	:   _m0(0.0f), _m1(0.0f), _m2(0.0f), _m3(0.0f),
	    _m4(0.0f), _m5(0.0f), _m6(0.0f), _m7(0.0f),
	    _m8(0.0f), _m9(0.0f), _m10(0.0f), _m11(0.0f),
	    _m12(0.0f), _m13(0.0f), _m14(0.0f), _m15(0.0f)
{
}

// Special constructor
Matrix::Matrix(const Vect& vect0, const Vect& vect1, const Vect& vect2, const Vect& vect3)
	: v0(vect0), v1(vect1), v2(vect2), v3(vect3)
{
}

// Copy constructor
Matrix::Matrix(const Matrix& matrixIn)
	//: v0(matrixIn.v0), v1(matrixIn.v1), v2(matrixIn.v2), v3(matrixIn.v3)
{
	v0 = matrixIn.v0;
	v1 = matrixIn.v1;
	v2 = matrixIn.v2;
	v3 = matrixIn.v3;
}

Matrix& Matrix::operator= (const Matrix& rhs)
{
    if (this != &rhs)
	{
        this->v0 = rhs.v0;
        this->v1 = rhs.v1;
        this->v2 = rhs.v2;
        this->v3 = rhs.v3;
	}

    return *this;
};

// Matrix set function
void Matrix::setIdentity()
{
	memset(this, 0, sizeof(Matrix));
	this->_m0 = 1.0f;
	this->_m5 = 1.0f;
	this->_m10 = 1.0f;
	this->_m15 = 1.0f;
};

// Matrix set function
void Matrix::setZero()
{
	memset(this, 0, sizeof(Matrix));
};

// Set translation matrix
void Matrix::setTrans(const float xTrans, const float yTrans, const float zTrans)
{
    this->setIdentity();
    this->_m12 = xTrans;
    this->_m13 = yTrans;
    this->_m14 = zTrans;
}

// Set scale matrix
void Matrix::setScale(const float xScale, const float yScale, const float zScale)
{
    this->setZero();
    this->_m0 = xScale;
    this->_m5 = yScale;
    this->_m10 = zScale;
    this->_m15 = 1.0f;
}

void Matrix::setRotXYZ(const float rotxIn, const float rotyIn, const float rotzIn)
{
	// Set up matrices for rotating aroudnd x, y, and z axes
	Matrix rotX, rotY, rotZ;
	rotX.setIdentity();
	rotY.setIdentity();
	rotZ.setIdentity();

	// X axis matrix
	rotX._m5 = cosf(rotxIn);
	rotX._m6 = sinf(rotxIn);
	rotX._m9 = -rotX._m6;
	rotX._m10 = rotX._m5;

	// Y axis matrix
	rotY._m0 = cosf(rotyIn);
	rotY._m8 = sinf(rotyIn);
	rotY._m2 = -rotY._m8;
	rotY._m10 = rotY._m0;

	// Z axis matrix
	rotZ._m0 = cosf(rotzIn);
	rotZ._m1 = sinf(rotzIn);
	rotZ._m4 = -rotZ._m1;
	rotZ._m5 = rotZ._m0;

	*this = rotX * rotY * rotZ;
};

void Matrix::setRotAxisAngle(const Vect& axisIn, const float angleIn)
{
	const Vect vectNorm = axisIn.getNorm();

	const float cosine = cosf(angleIn);
	const float sine = sinf(angleIn);
	const float uIn = vectNorm[0];
	const float vIn = vectNorm[1];
	const float wIn = vectNorm[2];

	const float uSQ = uIn * uIn;
	const float vSQ = vIn * vIn;
	const float wSQ = wIn * wIn;

	this->_m0 = cosine + (1 - cosine) * uSQ;
	this->_m1 = (1 - cosine) * uIn * vIn + sine * wIn;
	this->_m2 = (1 - cosine) * uIn * wIn - sine * vIn;
	this->_m3 = 0.0f;

	this->_m4 = (1 - cosine) * uIn * vIn - sine * wIn;
	this->_m5 = cosine + (1 - cosine) * vSQ;
	this->_m6 = (1 - cosine) * vIn * wIn + sine * uIn;
	this->_m7 = 0.0f;

	this->_m8 = (1 - cosine) * uIn * wIn + sine * vIn;
	this->_m9 = (1 - cosine) * vIn * wIn - sine * uIn;
	this->_m10 = cosine + (1 - cosine) * wSQ;
	this->_m11 = 0.0f;

	this->_m12 = 0.0f;
	this->_m13 = 0.0f;
	this->_m14 = 0.0f;
	this->_m15 = 1.0f;
};

void Matrix::setRow(const int indexIn, const Vect& vectIn)
{
	assert(indexIn >= 0 && indexIn <= 3);

	this->v[indexIn] = vectIn;
};

void Matrix::set(const Vect& vect0, const Vect& vect1, const Vect& vect2, const Vect& vect3)
{
    this->v0 = vect0;
    this->v1 = vect1;
    this->v2 = vect2;
    this->v3 = vect3;
}

Vect Matrix::getRow(const int indexIn) const
{
	assert(indexIn >= 0 && indexIn <= 3);

	return this->v[indexIn];
}

void Matrix::T()
{
    Vect col0(_m0, _m4, _m8, _m12);
    Vect col1(_m1, _m5, _m9, _m13);
    Vect col2(_m2, _m6, _m10, _m14);
    Vect col3(_m3, _m7, _m11, _m15);

    this->set(col0, col1, col2, col3);
}

Matrix Matrix::getT() const
{
    Vect col0(_m0, _m4, _m8, _m12);
    Vect col1(_m1, _m5, _m9, _m13);
    Vect col2(_m2, _m6, _m10, _m14);
    Vect col3(_m3, _m7, _m11, _m15);

    return Matrix(col0, col1, col2, col3);
}

// Determinant of matrix
float Matrix::det() const
{
	float det0 = _m0* (_m5*(_m10*_m15 - _m11*_m14) - _m6*(_m9*_m15 - _m11*_m13) + _m7*(_m9*_m14 - _m10*_m13));
	float det1 = -_m1* (_m4*(_m10*_m15 - _m11*_m14) - _m6*(_m8*_m15 - _m11*_m12) + _m7*(_m8*_m14 - _m10*_m12));
	float det2 = _m2* (_m4*(_m9*_m15 - _m11*_m13) - _m5*(_m8*_m15 - _m11*_m12) + _m7*(_m8*_m13 - _m9*_m12));
	float det3 = -_m3* (_m4*(_m9*_m14 - _m10*_m13) - _m5*(_m8*_m14 - _m10*_m12) + _m6*(_m8*_m13 - _m9*_m12));

	return det0 + det1 + det2 + det3;
}

Matrix Matrix::getInv() const
{
	Matrix b;
	b._m0 = _m6*_m11*_m13 - _m7*_m10*_m13 + _m7*_m9*_m14 - _m5*_m11*_m14 - _m6*_m9*_m15 + _m5*_m10*_m15;
	b._m1 = _m3*_m10*_m13 - _m2*_m11*_m13 - _m3*_m9*_m14 + _m1*_m11*_m14 + _m2*_m9*_m15 - _m1*_m10*_m15;
	b._m2 = _m2*_m7*_m13 - _m3*_m6*_m13 + _m3*_m5*_m14 - _m1*_m7*_m14 - _m2*_m5*_m15 + _m1*_m6*_m15;
	b._m3 = _m3*_m6*_m9 - _m2*_m7*_m9 - _m3*_m5*_m10 + _m1*_m7*_m10 + _m2*_m5*_m11 - _m1*_m6*_m11;
	b._m4 = _m7*_m10*_m12 - _m6*_m11*_m12 - _m7*_m8*_m14 + _m4*_m11*_m14 + _m6*_m8*_m15 - _m4*_m10*_m15;
	b._m5 = _m2*_m11*_m12 - _m3*_m10*_m12 + _m3*_m8*_m14 - _m0*_m11*_m14 - _m2*_m8*_m15 + _m0*_m10*_m15;
	b._m6 = _m3*_m6*_m12 - _m2*_m7*_m12 - _m3*_m4*_m14 + _m0*_m7*_m14 + _m2*_m4*_m15 - _m0*_m6*_m15;
	b._m7 = _m2*_m7*_m8 - _m3*_m6*_m8 + _m3*_m4*_m10 - _m0*_m7*_m10 - _m2*_m4*_m11 + _m0*_m6*_m11;
	b._m8 = _m5*_m11*_m12 - _m7*_m9*_m12 + _m7*_m8*_m13 - _m4*_m11*_m13 - _m5*_m8*_m15 + _m4*_m9*_m15;
	b._m9 = _m3*_m9*_m12 - _m1*_m11*_m12 - _m3*_m8*_m13 + _m0*_m11*_m13 + _m1*_m8*_m15 - _m0*_m9*_m15;
	b._m10 = _m1*_m7*_m12 - _m3*_m5*_m12 + _m3*_m4*_m13 - _m0*_m7*_m13 - _m1*_m4*_m15 + _m0*_m5*_m15;
	b._m11 = _m3*_m5*_m8 - _m1*_m7*_m8 - _m3*_m4*_m9 + _m0*_m7*_m9 + _m1*_m4*_m11 - _m0*_m5*_m11;
	b._m12 = _m6*_m9*_m12 - _m5*_m10*_m12 - _m6*_m8*_m13 + _m4*_m10*_m13 + _m5*_m8*_m14 - _m4*_m9*_m14;
	b._m13 = _m1*_m10*_m12 - _m2*_m9*_m12 + _m2*_m8*_m13 - _m0*_m10*_m13 - _m1*_m8*_m14 + _m0*_m9*_m14;
	b._m14 = _m2*_m5*_m12 - _m1*_m6*_m12 - _m2*_m4*_m13 + _m0*_m6*_m13 + _m1*_m4*_m14 - _m0*_m5*_m14;
	b._m15 = _m1*_m6*_m8 - _m2*_m5*_m8 + _m2*_m4*_m9 - _m0*_m6*_m9 - _m1*_m4*_m10 + _m0*_m5*_m10;

	float f = 1.0f / (this->det());

	b *= f;

	return b;
}



const float Matrix::operator[](const int indexIn) const
{
	assert(indexIn >= 0 && indexIn <= 15);

	return this->_m[indexIn];
};

float& Matrix::operator[](const int indexIn)
{
	assert(indexIn >= 0 && indexIn <= 15);

	return this->_m[indexIn];
};

Matrix Matrix::operator+ (const Matrix& matrixIn) const
{
    Vect vect0(this->_m0 + matrixIn._m0, this->_m1 + matrixIn._m1, this->_m2 + matrixIn._m2, this->_m3 + matrixIn._m3);
    Vect vect1(this->_m4 + matrixIn._m4, this->_m5 + matrixIn._m5, this->_m6 + matrixIn._m6, this->_m7 + matrixIn._m7);
    Vect vect2(this->_m8 + matrixIn._m8, this->_m9 + matrixIn._m9, this->_m10 + matrixIn._m10, this->_m11 + matrixIn._m11);
    Vect vect3(this->_m12 + matrixIn._m12, this->_m13 + matrixIn._m13, this->_m14 + matrixIn._m14, this->_m15 + matrixIn._m15);

    return Matrix(vect0, vect1, vect2, vect3);
}

Matrix& Matrix::operator += (const Matrix& matrixIn)
{
    this->_m0 += matrixIn._m0;
    this->_m1 += matrixIn._m1;
    this->_m2 += matrixIn._m2;
    this->_m3 += matrixIn._m3;
    this->_m4 += matrixIn._m4;
    this->_m5 += matrixIn._m5;
    this->_m6 += matrixIn._m6;
    this->_m7 += matrixIn._m7;
    this->_m8 += matrixIn._m8;
    this->_m9 += matrixIn._m9;
    this->_m10 += matrixIn._m10;
    this->_m11 += matrixIn._m11;
    this->_m12 += matrixIn._m12;
    this->_m13 += matrixIn._m13;
    this->_m14 += matrixIn._m14;
    this->_m15 += matrixIn._m15;

    return *this;

}

Matrix Matrix::operator- (const Matrix& matrixIn) const
{
    Vect vect0(this->_m0 - matrixIn._m0, this->_m1 - matrixIn._m1, this->_m2 - matrixIn._m2, this->_m3 - matrixIn._m3);
    Vect vect1(this->_m4 - matrixIn._m4, this->_m5 - matrixIn._m5, this->_m6 - matrixIn._m6, this->_m7 - matrixIn._m7);
    Vect vect2(this->_m8 - matrixIn._m8, this->_m9 - matrixIn._m9, this->_m10 - matrixIn._m10, this->_m11 - matrixIn._m11);
    Vect vect3(this->_m12 - matrixIn._m12, this->_m13 - matrixIn._m13, this->_m14 - matrixIn._m14, this->_m15 - matrixIn._m15);

    return Matrix(vect0, vect1, vect2, vect3);
}

Matrix& Matrix::operator -= (const Matrix& matrixIn)
{
    this->_m0 -= matrixIn._m0;
    this->_m1 -= matrixIn._m1;
    this->_m2 -= matrixIn._m2;
    this->_m3 -= matrixIn._m3;
    this->_m4 -= matrixIn._m4;
    this->_m5 -= matrixIn._m5;
    this->_m6 -= matrixIn._m6;
    this->_m7 -= matrixIn._m7;
    this->_m8 -= matrixIn._m8;
    this->_m9 -= matrixIn._m9;
    this->_m10 -= matrixIn._m10;
    this->_m11 -= matrixIn._m11;
    this->_m12 -= matrixIn._m12;
    this->_m13 -= matrixIn._m13;
    this->_m14 -= matrixIn._m14;
    this->_m15 -= matrixIn._m15;

    return *this;
}

Matrix Matrix::operator* (const Matrix& matrixIn) const
{
    Matrix returnMatrix;

    returnMatrix._m0 = this->_m0 * matrixIn._m0 + this->_m1 * matrixIn._m4 + this->_m2 * matrixIn._m8 + this->_m3 * matrixIn._m12;
    returnMatrix._m1 = this->_m0 * matrixIn._m1 + this->_m1 * matrixIn._m5 + this->_m2 * matrixIn._m9 + this->_m3 * matrixIn._m13;
    returnMatrix._m2 = this->_m0 * matrixIn._m2 + this->_m1 * matrixIn._m6 + this->_m2 * matrixIn._m10 + this->_m3 * matrixIn._m14;
    returnMatrix._m3 = this->_m0 * matrixIn._m3 + this->_m1 * matrixIn._m7 + this->_m2 * matrixIn._m11 + this->_m3 * matrixIn._m15;

    returnMatrix._m4 = this->_m4 * matrixIn._m0 + this->_m5 * matrixIn._m4 + this->_m6 * matrixIn._m8 + this->_m7 * matrixIn._m12;
    returnMatrix._m5 = this->_m4 * matrixIn._m1 + this->_m5 * matrixIn._m5 + this->_m6 * matrixIn._m9 + this->_m7 * matrixIn._m13;
    returnMatrix._m6 = this->_m4 * matrixIn._m2 + this->_m5 * matrixIn._m6 + this->_m6 * matrixIn._m10 + this->_m7 * matrixIn._m14;
    returnMatrix._m7 = this->_m4 * matrixIn._m3 + this->_m5 * matrixIn._m7 + this->_m6 * matrixIn._m11 + this->_m7 * matrixIn._m15;
    
    returnMatrix._m8 = this->_m8 * matrixIn._m0 + this->_m9 * matrixIn._m4 + this->_m10 * matrixIn._m8 + this->_m11 * matrixIn._m12;
    returnMatrix._m9 = this->_m8 * matrixIn._m1 + this->_m9 * matrixIn._m5 + this->_m10 * matrixIn._m9 + this->_m11 * matrixIn._m13;
    returnMatrix._m10 = this->_m8 * matrixIn._m2 + this->_m9 * matrixIn._m6 + this->_m10 * matrixIn._m10 + this->_m11 * matrixIn._m14;
    returnMatrix._m11 = this->_m8 * matrixIn._m3 + this->_m9 * matrixIn._m7 + this->_m10 * matrixIn._m11 + this->_m11 * matrixIn._m15;

    returnMatrix._m12 = this->_m12 * matrixIn._m0 + this->_m13 * matrixIn._m4 + this->_m14 * matrixIn._m8 + this->_m15 * matrixIn._m12;
    returnMatrix._m13 = this->_m12 * matrixIn._m1 + this->_m13 * matrixIn._m5 + this->_m14 * matrixIn._m9 + this->_m15 * matrixIn._m13;
    returnMatrix._m14 = this->_m12 * matrixIn._m2 + this->_m13 * matrixIn._m6 + this->_m14 * matrixIn._m10 + this->_m15 * matrixIn._m14;
    returnMatrix._m15 = this->_m12 * matrixIn._m3 + this->_m13 * matrixIn._m7 + this->_m14 * matrixIn._m11 + this->_m15 * matrixIn._m15;

    return returnMatrix;
};

Matrix& Matrix::operator*= (const Matrix& matrixIn)
{
    Matrix matrix;
    matrix._m0 = this->_m0 * matrixIn._m0 + this->_m1 * matrixIn._m4 + this->_m2 * matrixIn._m8 + this->_m3 * matrixIn._m12;
    matrix._m1 = this->_m0 * matrixIn._m1 + this->_m1 * matrixIn._m5 + this->_m2 * matrixIn._m9 + this->_m3 * matrixIn._m13;
    matrix._m2 = this->_m0 * matrixIn._m2 + this->_m1 * matrixIn._m6 + this->_m2 * matrixIn._m10 + this->_m3 * matrixIn._m14;
    matrix._m3 = this->_m0 * matrixIn._m3 + this->_m1 * matrixIn._m7 + this->_m2 * matrixIn._m11 + this->_m3 * matrixIn._m15;

    matrix._m4 = this->_m4 * matrixIn._m0 + this->_m5 * matrixIn._m4 + this->_m6 * matrixIn._m8 + this->_m7 * matrixIn._m12;
    matrix._m5 = this->_m4 * matrixIn._m1 + this->_m5 * matrixIn._m5 + this->_m6 * matrixIn._m9 + this->_m7 * matrixIn._m13;
    matrix._m6 = this->_m4 * matrixIn._m2 + this->_m5 * matrixIn._m6 + this->_m6 * matrixIn._m10 + this->_m7 * matrixIn._m14;
    matrix._m7 = this->_m4 * matrixIn._m3 + this->_m5 * matrixIn._m7 + this->_m6 * matrixIn._m11 + this->_m7 * matrixIn._m15;
    
    matrix._m8 = this->_m8 * matrixIn._m0 + this->_m9 * matrixIn._m4 + this->_m10 * matrixIn._m8 + this->_m11 * matrixIn._m12;
    matrix._m9 = this->_m8 * matrixIn._m1 + this->_m9 * matrixIn._m5 + this->_m10 * matrixIn._m9 + this->_m11 * matrixIn._m13;
    matrix._m10 = this->_m8 * matrixIn._m2 + this->_m9 * matrixIn._m6 + this->_m10 * matrixIn._m10 + this->_m11 * matrixIn._m14;
    matrix._m11 = this->_m8 * matrixIn._m3 + this->_m9 * matrixIn._m7 + this->_m10 * matrixIn._m11 + this->_m11 * matrixIn._m15;

    matrix._m12 = this->_m12 * matrixIn._m0 + this->_m13 * matrixIn._m4 + this->_m14 * matrixIn._m8 + this->_m15 * matrixIn._m12;
    matrix._m13 = this->_m12 * matrixIn._m1 + this->_m13 * matrixIn._m5 + this->_m14 * matrixIn._m9 + this->_m15 * matrixIn._m13;
    matrix._m14 = this->_m12 * matrixIn._m2 + this->_m13 * matrixIn._m6 + this->_m14 * matrixIn._m10 + this->_m15 * matrixIn._m14;
    matrix._m15 = this->_m12 * matrixIn._m3 + this->_m13 * matrixIn._m7 + this->_m14 * matrixIn._m11 + this->_m15 * matrixIn._m15;

    *this = matrix;

    return *this;
}

Matrix operator * (const float s, const Matrix& matrixIn)
{
    Vect vect0(s * matrixIn._m0, s * matrixIn._m1, s * matrixIn._m2, s * matrixIn._m3);
    Vect vect1(s * matrixIn._m4, s * matrixIn._m5, s * matrixIn._m6, s * matrixIn._m7);
    Vect vect2(s * matrixIn._m8, s * matrixIn._m9, s * matrixIn._m10, s * matrixIn._m11);
    Vect vect3(s * matrixIn._m12, s * matrixIn._m13, s * matrixIn._m14, s * matrixIn._m15);

    return Matrix(vect0, vect1, vect2, vect3);
}

Matrix Matrix::operator* (const float s) const
{
    Vect vect0(s * this->_m0, s * this->_m1, s * this->_m2, s * this->_m3);
    Vect vect1(s * this->_m4, s * this->_m5, s * this->_m6, s * this->_m7);
    Vect vect2(s * this->_m8, s * this->_m9, s * this->_m10, s * this->_m11);
    Vect vect3(s * this->_m12, s * this->_m13, s * this->_m14, s * this->_m15);

    return Matrix(vect0, vect1, vect2, vect3);
}

Matrix& Matrix::operator*= (const float s)
{
    this->_m0 *= s;
    this->_m1 *= s;
    this->_m2 *= s;
    this->_m3 *= s;
    this->_m4 *= s;
    this->_m5 *= s;
    this->_m6 *= s;
    this->_m7 *= s;
    this->_m8 *= s;
    this->_m9 *= s;
    this->_m10 *= s;
    this->_m11 *= s;
    this->_m12 *= s;
    this->_m13 *= s;
    this->_m14 *= s;
    this->_m15 *= s;

    return *this;
}

// Quat set function
void Matrix::set(const Quat& quatIn)
{
	const float qx = quatIn[0];
	const float qy = quatIn[1];
	const float qz = quatIn[2];
	const float qw = quatIn[3];

	this->_m0 = 1 - 2 * (qy*qy + qz*qz);
	this->_m1 = 2 * (qx*qy + qw*qz);
	this->_m2 = 2 * (qx*qz - qw*qy);
	this->_m3 = 0.0f;

	this->_m4 = 2 * (qx*qy - qw*qz);
	this->_m5 = 1 - 2 * (qx*qx + qz*qz);
	this->_m6 = 2 * (qy*qz + qw*qx);
	this->_m7 = 0.0f;

	this->_m8 = 2 * (qx*qz + qw*qy);
	this->_m9 = 2 * (qy*qz - qw*qx);
	this->_m10 = 1 - 2 * (qx*qx + qy*qy);
	this->_m11 = 0.0f;

	this->_m12 = 0.0f;
	this->_m13 = 0.0f;
	this->_m14 = 0.0f;
	this->_m15 = 1.0f;
};

