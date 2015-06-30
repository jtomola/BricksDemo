#ifndef MOTION_BLUR_H
#define MOTION_BLUR_H

#include "D3DHeader.h"
#include <time.h>
#include <stdlib.h>

struct CBBlur
{
	int startIndex;
	int stopIndex;
	int numTextures;
	float percentageEach;

	CBBlur() : startIndex(0), stopIndex(0), numTextures(0), percentageEach(0) {};
};

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
	CBBlur						cbBlur;
	ID3D11Texture2D*			blurTextures;
	ID3D11ShaderResourceView*	blurSRVs;
	ID3D11Texture2D*			mainRenderTexture;
	ID3D11RenderTargetView*		mainRenderView;
	ID3D11Texture2D*			depthTexture;
	ID3D11DepthStencilView*		depthView;
	ID3D11RenderTargetView*		currRTV;
	ID3D11Buffer*				blurInfoBuffer;

	// For drawing to back buffer
	ID3D11Buffer*				vertBuffer;
	ID3D11Buffer*				indexBuffer;
	// Shader Info
	ID3D11VertexShader*			vShader;
	ID3D11PixelShader*			pShader;
	ID3D11InputLayout*			inputLayout;
	float						blurTime;
	clock_t						lastTime;
	int							currIndex;
	int							numTextures;
	int							initCount;
};

#endif