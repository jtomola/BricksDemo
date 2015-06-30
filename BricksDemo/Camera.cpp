#include <math.h>
#include "Camera.h"

// Default constructor 
Camera::Camera()
	:	vUp(), vDir(), vRight(), vPos(),
		nearDist(0.0f), farDist(0.0f), fovY(0.0f), aspectRatio(0.0f),
		nearHeight(0.0f), nearWidth(0.0f), farHeight(0.0f), farWidth(0.0f),
		viewportX(0), viewportY(0), viewportWidth(0), viewportHeight(0),
		viewMatrix(), projMatrix()
{
};

// Critical info for perspective matrix
// Field of view, aspect ration, near plane, and far plane
void Camera::setPerspective(const float fov_y, const float aspect, const float nearD, const float farD)
{
    // Set data
    this->fovY = fov_y;
	this->aspectRatio = aspect;
    this->nearDist = nearD;
    this->farDist = farD;

	// Calculate width and height of the near and far planes
    this->nearHeight = 2.0f * tanf( (this->fovY * MATH_PI/180.0f ) * 0.5f) * this->nearDist;
    this->nearWidth = this->nearHeight * this->aspectRatio;

    this->farHeight = 2.0f * tanf( (this->fovY * MATH_PI/180.0f ) * 0.5f) * this->farDist;
    this->farWidth = this->farHeight * this->aspectRatio;
};

// Pass through function to set up view port - screen sub window
void Camera::setViewport(const int inX, const int inY, const int width, const int height)
{
    // Set data
    this->viewportX = inX;
    this->viewportY = inY;
    this->viewportWidth = width;
    this->viewportHeight = height;
};

// Set camera's position and orientation with 3 vectors
void Camera::setOrientAndPosition(const Vect& upIn, const Vect& lookIn, const Vect& posIn)
{
    // lookIn is point in world space camera is looking at
    // get direction vector for this, and normalize
    this->vDir = posIn - lookIn;
    this->vDir.norm();

    // Get right vector using cross product (and normalize it)
    this->vRight = upIn.cross(this->vDir);
    this->vRight.norm();

    // Get up vector by crossing direction and right vectors
    // then normalize
    this->vUp = this->vDir.cross(this->vRight);
    this->vUp.norm();

    this->vPos = posIn;
};

// Update projection matrix
void Camera::privUpdateProjectionMatrix()
{
    this->projMatrix[0] = 2.0f * this->nearDist / this->nearWidth;
    this->projMatrix[1] = 0.0f;
    this->projMatrix[2] = 0.0f;
    this->projMatrix[3] = 0.0f;
    
    this->projMatrix[4] = 0.0f;
    this->projMatrix[5] = 2.0f * this->nearDist / this->nearHeight;
    this->projMatrix[6] = 0.0f;
    this->projMatrix[7] = 0.0f;

    this->projMatrix[8] = 0.0f;
    this->projMatrix[9] = 0.0f;
    this->projMatrix[10] = (this->farDist) / (this->nearDist - this->farDist);
    this->projMatrix[11] = -1.0f;

    this->projMatrix[12] = 0.0f;
    this->projMatrix[13] = 0.0f;
    this->projMatrix[14] = (this->nearDist * this->farDist) / (this->nearDist - this->farDist);
    this->projMatrix[15] = 0.0f;
};

// Update view matrix
// Assumes vUp, vDir, and vRight are all unit vectors
void Camera::privUpdateViewMatrix()
{
    this->viewMatrix[0] = this->vRight[0];
    this->viewMatrix[1] = this->vUp[0];
    this->viewMatrix[2] = this->vDir[0];
    this->viewMatrix[3] = 0.0f;

    this->viewMatrix[4] = this->vRight[1];
    this->viewMatrix[5] = this->vUp[1];
    this->viewMatrix[6] = this->vDir[1];
    this->viewMatrix[7] = 0.0f;

    this->viewMatrix[8] = this->vRight[2];
    this->viewMatrix[9] = this->vUp[2];
    this->viewMatrix[10] = this->vDir[2];
    this->viewMatrix[11] = 0.0f;

    this->viewMatrix[12] = -vPos.dot(vRight);
    this->viewMatrix[13] = -vPos.dot(vUp);
    this->viewMatrix[14] = -vPos.dot(vDir);
    this->viewMatrix[15] = 1.0f;
};

// Update every part of the camera
void Camera::updateCamera(void)
{
    // Update projection matrix
    this->privUpdateProjectionMatrix();

    // Update view matrix
    this->privUpdateViewMatrix();
};

// Grab view Matrix
Matrix& Camera::getViewMatrix()
{
    return this->viewMatrix;
};

// Grab projection matrix
Matrix& Camera::getProjMatrix()
{
    return this->projMatrix;
};

// Rotate camera around global Y axis
void Camera::rotateYGlobal(const float Value)
{
	Matrix Rot;
	Rot.setRotXYZ(0.0f, Value, 0.0f);
    this->vUp *= Rot; 
    this->vDir *= Rot;
    this->vRight *= Rot;
};

// Rotate camera around local X axis
void Camera::rotateXLocal(const float Value)
{
	Matrix Rot;
	Rot.setRotAxisAngle(this->vRight, Value);
    this->vUp *= Rot;
    this->vDir *= Rot;
};