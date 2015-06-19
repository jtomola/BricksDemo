#ifndef DEMO_H
#define DEMO_H

#include "Windows.h"
#include "D3DHeader.h"

// This class represents the whole brick demo. It initializes Direct3D, then handles all gameplay and rendering
class Demo
{
public:
	Demo();
	~Demo();

	static void Initialize(HINSTANCE hInstance, int nCmdShow);

	static void Run();
	static void Update();
	static void Draw();

	// Accessors for window, D3D device, and D3D device context
	static ID3D11Device* GetDevice();
	static ID3D11DeviceContext* GetDeviceContext();
	static HWND GetWindow();

	void Quit();

private:
	// Make this a singleton
	static Demo *privGetInstance();

	HWND privCreateGraphicsWindow(HINSTANCE hInstance, int nCmdShow, const char* windowName, const int Width, const int Height);
	void privInitDevice();
	void privShutDownDevice();
	void privCreateShader();
	void privDestroyShader();

	HWND						window;
	IDXGISwapChain*				swapChain;
	ID3D11Device*				device;
	ID3D11DeviceContext*		deviceCon;
	ID3D11RenderTargetView*		backBufferView;
	ID3D11Texture2D*			depthTexture;
	ID3D11DepthStencilView*		depthView;

	// Shader Info
	ID3D11VertexShader*			vShader;
	ID3D11PixelShader*			pShader;
	ID3D11InputLayout*			inputLayout;
	ID3D11RasterizerState*		rastState;

	bool running;
};


#endif
