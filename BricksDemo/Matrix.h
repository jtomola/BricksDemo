#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include "Vect.h"

class Matrix
{
public:
	friend class Vect;

    // Constructors
    explicit Matrix();
    explicit Matrix(const Vect& vect0, const Vect& vect1, const Vect& vect2, const Vect& vect3);

public:
    Matrix(const Matrix& matrixIn);
    
    // Assignment operator
    Matrix& operator=( const Matrix& rhs);

    // Destructor
	~Matrix() { }

    // Matrix set functions
    void setZero();
    void setIdentity();
    void setTrans(const float xTrans, const float yTrans, const float zTrans);
    void setScale(const float xScale, const float yScale, const float zScale);
    void setRotXYZ(const float rotxIn, const float rotyIn, const float rotzIn);
    void setRotAxisAngle(const Vect& axisIn, const float angleIn);

    // Get and set functions for rows
    void set(const Vect& vect0, const Vect& vect1, const Vect& vect2, const Vect& vect3);
    void setRow(const int indexIn, const Vect& vectIn);
    Vect getRow(const int indexIn) const;

    // Special matrix functions
    void T();
    Matrix getT() const;

    // Overloaded bracket operators for getting
    const float operator[](const int indexIn) const;

    // Overloaded bracket operators for setting
    float& operator[](const int indexIn);

    // Matrix addition and subtraction
    Matrix operator+ (const Matrix& matrixIn) const;
    Matrix& operator+= (const Matrix& matrixIn);
    Matrix operator- (const Matrix& matrixIn) const;
    Matrix& operator-= (const Matrix& matrixIn);

    // Matrix multiplication
    Matrix operator* (const Matrix& matrixIn) const;
    Matrix& operator*= (const Matrix& matrixIn);

    // Multiplication by scalar
    friend Matrix operator * (const float s, const Matrix& matrixIn);
    Matrix operator* (const float s) const;
    Matrix& operator*= (const float s);

private:

	union {

		struct 
		{
			Vect v0;
			Vect v1;
			Vect v2;
			Vect v3;
		};

		struct
		{
			Vect v[4];
		};

		struct 
		{
			float _m0;
			float _m1;
			float _m2;
			float _m3;
			float _m4;
			float _m5;
			float _m6;	
			float _m7;
			float _m8;
			float _m9;
			float _m10;
			float _m11;
			float _m12;
			float _m13;
			float _m14;
			float _m15;
		};

		struct
		{
			float _m[16];
		};

	};
};

#endif
