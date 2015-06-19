#include "D3DHeader.h"
#include "Demo.h"
#include <stdio.h>
#include "Vect.h"
#include "Matrix.h"
#include "Camera.h"

// Callback needed to handle Window messages
LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Constructor
Demo::Demo()
	:	window(0), swapChain(0), device(0), deviceCon(0),
		backBufferView(0), depthTexture(0), depthView(0),
		vShader(0), pShader(0), inputLayout(0), rastState(0),
		running(0)
{
};

// Destructor
Demo::~Demo()
{
};

// Get singleton instance
Demo* Demo::privGetInstance()
{
	// Singleton storage
	static Demo Instance;
	return &Instance;
};

ID3D11Device* Demo::GetDevice()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->device;
};

ID3D11DeviceContext* Demo::GetDeviceContext()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->deviceCon;
};

HWND Demo::GetWindow()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->window;
};

// Initialize the engine
void Demo::Initialize(HINSTANCE hInstance, int nCmdShow)
{
	// Grab instance
	Demo* pDemo = Demo::privGetInstance();

	// Create window
	pDemo->window = pDemo->privCreateGraphicsWindow(hInstance, nCmdShow, "Bricks Demo", GAME_WIDTH, GAME_HEIGHT);

	// Initialize Direct3D
	pDemo->privInitDevice();
	pDemo->privCreateShader();
};

void Demo::Update()
{
	Demo* pDemo = Demo::privGetInstance();
	UNUSED(pDemo);
};

void Demo::Draw()
{
	Demo* pDemo = Demo::privGetInstance();
	UNUSED(pDemo);

	// Clear background
	float color[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	pDemo->deviceCon->ClearRenderTargetView(pDemo->backBufferView, (const float*)&color[0]);

	// Clear the depth buffer to 1.0 (max depth)
	pDemo->deviceCon->ClearDepthStencilView(pDemo->depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	pDemo->swapChain->Present(0, 0);
};

// Run the demo
void Demo::Run()
{
	Camera cam;
	cam.setViewport(0, 0, GAME_WIDTH, GAME_HEIGHT);
	cam.setPerspective(35.0f, float(GAME_WIDTH) / float(GAME_HEIGHT), 1.0f, 4500.0f);
	cam.setOrientAndPosition(Vect(0.0f, 1.0f, 0.0f), Vect(0.0f, 0.0f, -10.0f), Vect(0.0f, 0.0f, 300.0f));
	cam.updateCamera();

	// Get the instance
	Demo *p = Demo::privGetInstance();

	p->running = true;

	// Main message loop
	MSG msg = { 0 };

	// Game Loop until an exit
	while (p->running)
	{
		// Check messages
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (WM_QUIT == msg.message)
			{
				p->Quit();
				continue;
			}
		}
		else
		{
			// called once per frame, update data, transformations, etc
			p->Update();

			// called once per frame, only do rendering here
			p->Draw();
		}
	}

	p->privDestroyShader();
	p->privShutDownDevice();
};

// Quit
void Demo::Quit()
{
	// Grab instance
	Demo* pDemo = Demo::privGetInstance();

	pDemo->running = false;
};

// Create window
HWND Demo::privCreateGraphicsWindow(HINSTANCE hInstance, int nCmdShow, const char* windowName, const int Width, const int Height)
{
	// Register class
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = "WindowClass1";
	if (!RegisterClassEx(&wcex))
	{
		printf("Registering window class failed...\n");
		assert(0);
	}

	// Create window
	RECT rc = { 0, 0, Width, Height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hWnd = CreateWindow("WindowClass1", windowName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!hWnd)
	{
		printf("Creating window failed...\n");
		assert(0);
	}

	ShowWindow(hWnd, nCmdShow);

	return hWnd;
};

// Callback called every time the application receives a message
LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		// Track mouse movement
	case WM_MOUSEMOVE:
		break;
		//xPos = GET_X_LPARAM(lParam);
		//yPos = GET_Y_LPARAM(lParam);
		//InputMan::SetMousePos(xPos, yPos);

		// Need to capture it outside of window
	case WM_LBUTTONDOWN:
		SetCapture(Demo::GetWindow());
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		break;

	case WM_RBUTTONDOWN:
		SetCapture(Demo::GetWindow());
		break;

	case WM_RBUTTONUP:
		SetCapture(Demo::GetWindow());
		break;


	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void Demo::privInitDevice()
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                  
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  
	scd.BufferDesc.Width = GAME_WIDTH;                  
	scd.BufferDesc.Height = GAME_HEIGHT;               
	scd.BufferDesc.Height = GAME_HEIGHT;              
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = this->window;                 
	scd.SampleDesc.Count = 1;                       
	scd.SampleDesc.Quality = 0;                    
	scd.Windowed = TRUE;                          
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&this->swapChain,
		&this->device,
		NULL,
		&this->deviceCon);

	if (result == E_FAIL)
	{
		printf("Direct3D Initialization failed...\n");
		assert(0);
	}


	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (result == E_FAIL)
	{
		printf("Grabbing back buffer failed...\n");
		assert(0);
	}

	// use the back buffer address to create the render target
	result = device->CreateRenderTargetView(pBackBuffer, NULL, &backBufferView);
	if (result == E_FAIL)
	{
		printf("Creating render target view failed...\n");
		assert(0);
	}
	pBackBuffer->Release();

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = GAME_WIDTH;
	descDepth.Height = GAME_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	result = device->CreateTexture2D(&descDepth, nullptr, &depthTexture);
	if (FAILED(result))
	{
		printf("Creating depth texture failed...\n");
		assert(0);
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	result = device->CreateDepthStencilView(depthTexture, &descDSV, &depthView);
	if (FAILED(result))
	{
		printf("Creating depth view failed...\n");
		assert(0);
	}

	// Set the render targets (back buffer of our swap chain and the depth texture we created)
	deviceCon->OMSetRenderTargets(1, &backBufferView, depthView);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = GAME_WIDTH;
	viewport.Height = GAME_HEIGHT;

	deviceCon->RSSetViewports(1, &viewport);
};

void Demo::privShutDownDevice()
{
	swapChain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

	if (deviceCon)
	{
		deviceCon->ClearState();
	}

	// close and release all existing D3D objects
	if (backBufferView) backBufferView->Release(); backBufferView = 0;
	if (depthView) depthView->Release(); depthView = 0;
	if (depthTexture) depthTexture->Release(); depthTexture = 0;
	if (swapChain) swapChain->Release(); swapChain = 0;
	if (device) device->Release(); device = 0;
	if (deviceCon) deviceCon->Release(); deviceCon = 0;
};

void Demo::privCreateShader()
{
	ID3DBlob* vShaderBlob = 0;
	ID3DBlob* pShaderBlob = 0;

	// Just one set of shaders - for displaying solid color meshes that are lit 
	// by a directional light

	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Skip optimization and add debug info if we're in debug configuration
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// Create the vertex shader
	ID3DBlob* pErrorBlob = nullptr;

	// Compile vertex shader
	hr = D3DCompileFromFile(L"FlatColorWithLight.hlsl", nullptr, nullptr, "VShader", "vs_4_0",
		dwShaderFlags, 0, &vShaderBlob, &pErrorBlob);

	// Check for success
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			printf("Vertex shader compilation failed.\n");
			printf(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
			pErrorBlob = nullptr;
			assert(0);
		}
	}
	if (pErrorBlob) pErrorBlob->Release();

	// Create the vertex shader
	hr = this->device->CreateVertexShader(vShaderBlob->GetBufferPointer(), vShaderBlob->GetBufferSize(), nullptr, &vShader);
	if (FAILED(hr))
	{
		printf("Vertex shader creation failed.\n");
		vShaderBlob->Release();
		assert(0);
	}

	pErrorBlob = 0;
	// Compile pixel shader
	hr = D3DCompileFromFile(L"FlatColorWithLight.hlsl", nullptr, nullptr, "PShader", "ps_4_0",
		dwShaderFlags, 0, &pShaderBlob, &pErrorBlob);

	// Check for success
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			printf(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			printf("Pixel Shader compilation failed.\n");
			pErrorBlob->Release();
			pErrorBlob = nullptr;
			assert(0);
		}
	}
	if (pErrorBlob) pErrorBlob->Release();

	// Create the pixel shader
	hr = this->device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &pShader);
	if (FAILED(hr))
	{
		printf("Pixel shader creation failed.\n");
		pShaderBlob->Release();
		assert(0);
	}

	//Input Layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	hr = this->device->CreateInputLayout(layout, ARRAYSIZE(layout), vShaderBlob->GetBufferPointer(),
		vShaderBlob->GetBufferSize(), &inputLayout);

	// Release our blobs
	vShaderBlob->Release();
	pShaderBlob->Release();

	if (FAILED(hr))
	{
		printf("Input layout creation failed...\n");
		assert(0);
	}

	// Now create the rasterizer state
	D3D11_RASTERIZER_DESC descRast;
	ZeroMemory(&descRast, sizeof(descRast));
	descRast.FillMode = D3D11_FILL_SOLID;
	descRast.CullMode = D3D11_CULL_BACK;
	descRast.FrontCounterClockwise = true;
	descRast.DepthClipEnable = true;
	descRast.MultisampleEnable = false;

	hr = this->device->CreateRasterizerState(&descRast, &rastState);
	if (FAILED(hr))
	{
		printf("Rasterizer state creation failed...\n");
		assert(0);
	}
};

void Demo::privDestroyShader()
{
	if (rastState != 0) { rastState->Release(); rastState = 0; }
	if (inputLayout != 0) { inputLayout->Release(); inputLayout = 0; }
	if (vShader != 0) { vShader->Release(); vShader = 0; }
	if (pShader != 0) { pShader->Release(); pShader = 0; }
}
