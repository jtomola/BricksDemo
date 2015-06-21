#ifndef MATH_VECT_H
#define MATH_VECT_H

#define MATH_PI 3.1415926535f

class Matrix;

class Vect
{
public:
	// Constructors
	Vect();
	Vect(const float inX, const float inY, const float inZ, const float inW = 1.0f);
	Vect(const Vect& vectIn);

public:

	// Destructor
	~Vect() { }

	// Assignment Operator
	Vect& operator=(const Vect& rhs);

	// Set functions
	void set(const float inX, const float inY, const float inZ, const float inW = 1.0f);
	void set(const Vect& rhs);

	// Accessors by number
	const float operator[](const unsigned int indexIn) const;
	float& operator[](const unsigned int indexIn);

	// Overloading addition and subtraction
	Vect operator+ (const Vect& vectIn) const;
	Vect& operator+= (const Vect& vectIn);
	Vect operator- (const Vect& vectIn) const;
	Vect& operator-= (const Vect& vectIn);

	// Overloading multiplication by scalar
	friend Vect operator * (const float s, const Vect& vectIn);
	Vect operator* (const float s) const;
	Vect& operator*= (const float s);

	// Multiplication by Matrix
	Vect operator*(const Matrix& matrixIn) const;
	Vect& operator*=(const Matrix& matrixIn);

	// Special vector functions
	float dot(const Vect& vectIn) const;
	Vect cross(const Vect& vectIn) const;
	void norm();
	Vect getNorm() const;
	float mag() const;
	float magSqr() const;

private:
	union {
		float v[4];

		struct {
			float vx;
			float vy;
			float vz;
			float vw;
		};
	};
};


#endif