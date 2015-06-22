#ifndef QUAT_H
#define QUAT_H

#include "Matrix.h"
#include "Vect.h"

class Quat 
{
public:
    // Constructors
    Quat();
    Quat(const Quat& quatIn);
    Quat(const float qxIn, const float qyIn, const float qzIn, const float qwIn); 

    // Assignment operator
    Quat& operator= (const Quat& rhs);

    // Destructor
	~Quat() { }

    // Set functions
    void set(const float qxIn, const float qyIn, const float qzIn, const float qwIn); 
    void set(const Quat& quatIn);
    void setRotXYZ(const float rotxIn, const float rotyIn, const float rotzIn);
	void setAxisAngle(const Vect& axisIn, const float angleIn);

	// Accessors to retrieve components of vector
	const float operator[](const int indexIn) const;

	// Same for setting
	float& operator[](const int indexIn);

    // Overloading multiplication
    Quat operator* (const Quat& quatIn) const;

private:

	// anonymous union
	union 
	{
		// anonymous struct
		struct 
		{
			float qx;
			float qy;
			float qz;
			float qw;
		};

		struct
		{
			float q[4];
		};

        struct
		{
            Vect qVect;
		};
	};


};



#endif // #ifndef QUAT_H

/**** END of Quat.h ********************************************************/
