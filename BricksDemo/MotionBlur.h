#ifndef MOTION_BLUR_H
#define MOTION_BLUR_H

#include "D3DHeader.h"
#include <time.h>
#include <stdlib.h>

// This is the a struct with info we will pass to shader
struct CBBlur
{
	// Indicates how many and which past frames to blur together
	int startIndex;
	int numUsed;
	int numTextures;
	float percentageEach;

	CBBlur() : startIndex(0), numUsed(0), numTextures(0), percentageEach(0) {};
};

// Class with knowledge to blur past frames together
class MotionBlur
{
public:
	MotionBlur();
	~MotionBlur();

	void initialize(const int numTexturesIn);
	void destroy();
	void draw();
	void setBlurTime(const float timeIn);
	void resetInitCount();

public:
	// Our depth texture and normal textures that hold past frames
	CBBlur						cbBlur;
	ID3D11Texture2D*			blurTextures;
	ID3D11ShaderResourceView*	blurSRVs;
	ID3D11Texture2D*			mainRenderTexture;
	ID3D11RenderTargetView*		mainRenderView;
	ID3D11Texture2D*			depthTexture;
	ID3D11DepthStencilView*		depthView;
	ID3D11RenderTargetView*		currRTV;
	ID3D11Buffer*				blurInfoBuffer;

	// Vertex and index buffer - just a rectangle covering whole screen
	// Used for drawing our blurred image to back buffer
	ID3D11Buffer*				vertBuffer;
	ID3D11Buffer*				indexBuffer;

	// Shader Info
	ID3D11VertexShader*			vShader;
	ID3D11PixelShader*			pShader;
	ID3D11InputLayout*			inputLayout;

	// Info about which frames to blur
	float						blurTime;   // total time into past that should be blurred together
	clock_t						lastTime;   // time of last frame
	int							currIndex;  // current frame index in our texture array
	int							numTextures;// total number of textures in our array
	int							initCount;
	bool						blurOn;
};

#endif