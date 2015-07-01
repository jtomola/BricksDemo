#ifndef DEMO_H
#define DEMO_H

#include "Windows.h"
#include "D3DHeader.h"
#include "Vect.h"
#include "Matrix.h"
#include "Camera.h"
#include "Crosshair.h"
#include "Block.h"
#include "MotionBlur.h"

#define NUM_BRICKS 30

// This class represents the whole brick demo. It initializes Direct3D, then handles all gameplay and rendering
class Demo
{
public:
	Demo();
	~Demo();

	static void Initialize(HINSTANCE hInstance, int nCmdShow);
	static void Shutdown();

	static void Run();
	static void Update();
	static void Draw();

	// Accessors for window, D3D device, and D3D device context
	static ID3D11Device* GetDevice();
	static ID3D11DeviceContext* GetDeviceContext();
	static ID3D11Texture2D* GetBackBuffer();
	static ID3D11RenderTargetView* GetBackBufferView();
	static ID3D11DepthStencilView* GetDepthView();
	static HWND GetWindow();
	static Camera* GetCamera();

	// Set info to pass to shaders
	static void SetModelView(const Matrix& mvIn);
	static void SetColorInfo(const Vect& colorIn);

	void Quit();

private:
	// Make this a singleton
	static Demo *privGetInstance();
	
	// Helper functions
	void privMoveCrosshairs(const float elapsedTime);
	void privFireBullet(const float elapsedTime);
	void privCheckCollisions(const float elapsedTime);
	void privCheckSlowTime(const float elapsedTime);
	void privReset();

	// Set up window and Direct3D
	HWND privCreateGraphicsWindow(HINSTANCE hInstance, int nCmdShow, const char* windowName, const int Width, const int Height);
	void privInitDevice();
	void privShutDownDevice();

	// Create and destroy shaders
	void privCreateShader();
	void privDestroyShader();

	// Create and destroy buffers
	void privCreateBuffers();
	void privDestroyBuffers();

	// Struct with light info to pass to shaders
	struct LightStruct
	{
		Vect direction;
		Vect color;
	};

	// Camera and motion blur
	Camera						cam; 
	MotionBlur					motionBlur;

	// Our physics objects
	Block						ground;
	Block						bricks[NUM_BRICKS];
	Block						bullet;

	// Crosshairs
	Crosshair					crosshairX;
	Crosshair					crosshairY;

	// Window and D3D device info
	HWND						window;
	IDXGISwapChain*				swapChain;
	ID3D11Device*				device;
	ID3D11DeviceContext*		deviceCon;
	ID3D11Texture2D*			backBuffer;
	ID3D11RenderTargetView*		backBufferView;
	ID3D11Texture2D*			depthTexture;
	ID3D11DepthStencilView*		depthView;

	// Shader Info
	ID3D11VertexShader*			vShader;
	ID3D11PixelShader*			pShader;
	ID3D11InputLayout*			inputLayout;
	ID3D11RasterizerState*		rastState;

	// Constant Buffers
	ID3D11Buffer*				modelViewBuffer;
	ID3D11Buffer*				projectionBuffer;
	ID3D11Buffer*				lightBuffer;
	ID3D11Buffer*				colorBuffer;

	// Vertex and index buffers
	ID3D11Buffer*				vertBuffer;
	ID3D11Buffer*				indexBuffer;

	// Sampler state - needed for motion blur textures
	ID3D11SamplerState*			sampler;

	// Data for constant buffers
	Matrix						modelView;
	Matrix						projection;
	LightStruct					lightInfo;
	Vect						globalLightDir;
	Vect						color;

	// Variables to see if running, and if time is currently slowed
	float						slowTimer;
	bool						running;
	bool						timeSlowed;
};


#endif
