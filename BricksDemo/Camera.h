#ifndef CAMERA_H
#define CAMERA_H

#include "Vect.h"
#include "Matrix.h"

class Camera
{
public:
    // Default constructor
    Camera();

    // Setup a single camera
    void setPerspective(const float FieldOfView_Degs, const float AspectRatio, const float NearDist, const float FarDist);
    void setViewport(const int inX, const int inY, const int width, const int height);
    void setOrientAndPosition(const Vect& upIn, const Vect& lookIn, const Vect& posIn);

    // Update the camera
    void updateCamera(void);

    // Get matrices - for rendering
    Matrix& getViewMatrix();
    Matrix& getProjMatrix();

	// Rotate the camera (No longer used as camera is stationary)
	void rotateYGlobal(const float Value); // around global Y axis
    void rotateXLocal(const float Value); // around localXx axis

    // Camera direction vectors 
    Vect    vUp;        // Up vector
    Vect    vDir;       // Vector camera is facing
    Vect    vRight;     // derived from vUp and vDir
    Vect    vPos;       // Position of camera

private: 
    // private helper functions
    void privUpdateProjectionMatrix();
    void privUpdateViewMatrix();

public:
    // Frustum inputs
    float   nearDist;
    float   farDist;
    float   fovY;
    float   aspectRatio;

    // Width and height of near and far planes of frustum
    float   nearHeight; 
    float   nearWidth;
    float   farHeight;
    float   farWidth;

private:
    // Viewport specification
    int     viewportX;
    int     viewportY;
    int     viewportWidth;
    int     viewportHeight;

    // Projection and view Matrices
    Matrix viewMatrix;
    Matrix projMatrix;
};

#endif