#include "D3DHeader.h"
#include "Demo.h"
#include <stdio.h>
#include "Vect.h"
#include "Matrix.h"
#include "Camera.h"
#include <time.h>
#include "PhysicsContact.h"
#include "CollisionCheck.h"
#include <stdlib.h>

// Callback needed to handle Window messages
LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Constructor
Demo::Demo()
	:	cam(), motionBlur(), ground(), bricks(), bullet(), crosshairX(), crosshairY(),
		window(0), swapChain(0), device(0), deviceCon(0),
		backBuffer(0), backBufferView(0), depthTexture(0), depthView(0),
		vShader(0), pShader(0), inputLayout(0), rastState(0),
		modelViewBuffer(0), projectionBuffer(0), lightBuffer(0),
		colorBuffer(0), vertBuffer(0), indexBuffer(0), sampler(0),
		modelView(), projection(), lightInfo(), globalLightDir(), color(),
		running(false), timeSlowed(false)
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

// Adjust position of our crosshairs
void Demo::privMoveCrosshairs(const float elapsedTime)
{
	// Check whether right mouse button is pressed
	short rmb = GetKeyState(VK_RBUTTON);
	bool rmbPressed = (rmb & 0x80) != 0;

	// Get mouse position
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	int xPos = (int)cursorPos.x;
	int yPos = (int)cursorPos.y;

	// Get window coordinates
	RECT winSize;
	GetWindowRect(this->window, &winSize);

	static float mouseXPos = 0.0f, mouseYPos = 0.0f;

	mouseXPos = float(xPos - winSize.left) / float(winSize.right - winSize.left);
	mouseYPos = float(yPos - winSize.top) / float(winSize.bottom - winSize.top);

	// Move our crosshairs to correct screen position
	crosshairX.position[0] = crosshairY.position[0] = (mouseXPos - 0.5f) * 2.0f;
	crosshairX.position[1] = crosshairY.position[1] = -(mouseYPos - 0.5f) * 2.0f;
};

// Helper function to fire bullet (if necessary)
void Demo::privFireBullet(const float elapsedTime)
{
	// Delay between shots
	const float waitTime = 2.0f;
	
	static float currTime = waitTime;
	currTime += elapsedTime;

	// Check whether left mouse button is pressed
	short lmb = GetKeyState(VK_LBUTTON);
	bool lmbPressed = (lmb & 0x80) != 0;

	// Fire if button is pressed and the wait time has elapsed
	if (lmbPressed && (!bullet.active || currTime >= waitTime))
	{
		// Need to figure out our target
		float width = cam.nearWidth + (cam.farWidth - cam.nearWidth) * (490.0f - cam.nearDist) / (cam.farDist - cam.nearDist);
		float height = cam.nearHeight + (cam.farHeight - cam.nearHeight) * (490.0f - cam.nearDist) / (cam.farDist - cam.nearDist);

		// Use current crosshair positions (in screen coordinates)
		// and width and height of frustum at distance of blocks
		Vect target;
		target[0] = crosshairX.position[0] * 0.5f * width;
		target[1] = crosshairX.position[1] * 0.5f * height + 50.0f;
		target[2] = -490.0f;
		target[3] = 1.0f;

		currTime = 0.0f;

		// Set our velocity to be toward the target point
		this->bullet.position = this->cam.vPos;
		this->bullet.velocity = target - this->bullet.position;
		this->bullet.velocity.norm();
		this->bullet.velocity *= 1000.0f;
		this->bullet.rotation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
		this->bullet.angVelocity = Vect(0.0f, 0.0f, 0.0f);
		this->bullet.active = 1;
		this->bullet.gravityNow = false;
	}
}

// Check our collisions and handle them accordingly
void Demo::privCheckCollisions(const float timeIn)
{
	PhysicsContact contact;
	contact.Reset();

	// Check bullet and ground
	if (CheckColliding(ground, bullet, contact))
	{
		// Handle collision
		contact.CalculateData(timeIn);
		contact.ChangeVelocity();
		contact.ChangePosition();
		contact.Reset();
	}

	// Check all bricks against ground
	for (int i = 0; i < NUM_BRICKS; i++)
	{ 
		if (CheckColliding(bricks[i], ground, contact))
		{
			// Handle collision
			contact.CalculateData(timeIn);
			contact.ChangeVelocity();
			contact.ChangePosition();
			contact.Reset();
		}
	}

	// Check all bricks against bullet
	for (int i = 0; i < NUM_BRICKS; i++)
	{ 
		if (CheckColliding(bullet, bricks[i], contact))
		{
			contact.CalculateData(timeIn);

			// Time to have some fun with all blocks within certain distance of this collision
			// Launch those bricks upward with random angular velocity
			for (int k = 0; k < NUM_BRICKS; k++)
			{
				// use mag squared to avoid square root
				Vect diffPos = bricks[k].position - contact.contactPoint;
				float magSquared = diffPos.magSqr();
				
				if (magSquared < 1500.0f && bricks[k].position[1] >= bricks[i].position[1])
				{
					Vect velocityChange(diffPos[0] > 0 ? 30.0f : -30.0f, 200.0f, 0.0f);
					bricks[k].velocity += velocityChange;

					static int x = 987444303;
					srand(x);
					x += 5134;

					// Random angular velocity
					Vect angVelocityChange(0.0f, 0.0f, 0.0f);
					angVelocityChange[0] = (float)(rand() % 60 - 30);
					angVelocityChange[1] = (float)(rand() % 60 - 30);
					angVelocityChange[2] = (float)(rand() % 60 - 30);

					bricks[k].angVelocity += angVelocityChange;
				}
			}

			// Slow time on this collision
			if (!this->timeSlowed)
			{
				this->slowTimer = 0.0f;
				this->timeSlowed = true;
				this->motionBlur.blurOn = true;
			}
			contact.Reset();
			bullet.active = false;
			break;
		}
	}

	// Now check all bricks with each other
	for (int i = 0; i < NUM_BRICKS; i++)
	{
		for (int j = i + 1; j < NUM_BRICKS; j++)
		{
			if (CheckColliding(bricks[i], bricks[j], contact))
			{
				// Handle collision accordingly
				contact.CalculateData(timeIn);
				contact.ChangeVelocity();
				contact.ChangePosition();
				contact.Reset();
			}
		}
	}

	return;
};

// Check if time is slowed, and whether it should return to normal
void Demo::privCheckSlowTime(const float elapsedTime)
{
	if (!this->timeSlowed) return;

	slowTimer += elapsedTime;
	if (slowTimer > 6.0f)
	{
		slowTimer = 0.0f;
		timeSlowed = false;
		motionBlur.blurOn = false;
	}
}

void Demo::privReset()
{
	// Setup ground
	ground.color = Vect(0.0f, 0.4f, 0.0f, 1.0f);
	ground.position = Vect(0.0f, -2.5f, 0.0f);
	ground.scale = Vect(1000.0f, 5.0f, 3000.0f);
	ground.inverseMass = 0.0f;
	ground.CalcInertiaTensor();
	ground.CalculateDerivedData();

	// Setup crosshairs
	crosshairX.position = Vect(0.0f, 0.0f, 0.0000005f);
	crosshairX.scale = Vect(0.15f, 0.01f, 0.00000001f);
	crosshairY.position = Vect(0.0f, 0.0f, 0.0000005f);
	crosshairY.scale = Vect(0.01f, 0.20f, 0.00000001f);

	// Set the 4 colors for our bricks
	Vect colors[4];
	colors[0] = Vect(1.0f, 0.0f, 0.0f, 1.0f);
	colors[1] = Vect(0.0f, 1.0f, 0.0f, 1.0f);
	colors[2] = Vect(0.0f, 0.0f, 1.0f, 1.0f);
	colors[3] = Vect(1.0f, 1.0f, 0.0f, 1.0f);

	// Setup the bricks
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 6; j++)
		{ 
			int index = i * 6 + j;
			bricks[index].scale = Vect(20.0f, 20.0f, 20.f);
			bricks[index].color = colors[((j % 4) + i) % 4];
			bricks[index].position = Vect(-50.0f + 20.0f * j, 10.0f + 20.0f * i, -500.0f);
			bricks[index].velocity = Vect(0.0f, 0.0f, 0.0f);
			bricks[index].angVelocity = Vect(0.0f, 0.0f, 0.0f);
			bricks[index].rotation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
			bricks[index].inverseMass = 0.2f;
			bricks[index].gravityNow = false;
			bricks[index].CalcInertiaTensor();
		}
	}

	// Setup our bullet
	bullet.scale = Vect(2.0f, 2.0f, 2.f);
	bullet.color = Vect(0.0f, 0.0f, 0.0f, 1.0f);
	bullet.position = Vect(0.0f, 1000.0f, 0.0f);
	bullet.inverseMass = 0.5f;
	bullet.gravityNow = false;
	bullet.gravityEver = false;
	bullet.active = false;
	bullet.CalcInertiaTensor();

	// Turn off slow time and motion blur
	this->timeSlowed = false;
	this->motionBlur.blurOn = false;
};

// Get D3D device
ID3D11Device* Demo::GetDevice()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->device;
};

// Get D3D device context
ID3D11DeviceContext* Demo::GetDeviceContext()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->deviceCon;
};

// Get back buffer
ID3D11Texture2D* Demo::GetBackBuffer()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->backBuffer;
}

// Get render target view for back buffer
ID3D11RenderTargetView* Demo::GetBackBufferView()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->backBufferView;
}

// Get view for depth texture
ID3D11DepthStencilView* Demo::GetDepthView()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->depthView;
}

// Get handle for window
HWND Demo::GetWindow()
{
	Demo* pDemo = Demo::privGetInstance();
	return pDemo->window;
};

// Get our camera
Camera* Demo::GetCamera()
{
	Demo* pDemo = Demo::privGetInstance();
	return &pDemo->cam;
}

// Set modelview matrix (and push to shader)
void Demo::SetModelView(const Matrix& mvIn)
{
	// Grab instance
	Demo* pDemo = Demo::privGetInstance();

	pDemo->modelView = mvIn;
	pDemo->modelView.T();

	pDemo->deviceCon->UpdateSubresource(pDemo->modelViewBuffer, 0, nullptr, &pDemo->modelView, 0, 0);
}

// Set color info (and push to shader)
// (each block drawn with constant color)
void Demo::SetColorInfo(const Vect& colorIn)
{
	// Grab instance
	Demo* pDemo = Demo::privGetInstance();

	pDemo->color = colorIn;

	pDemo->deviceCon->UpdateSubresource(pDemo->colorBuffer, 0, nullptr, &pDemo->color, 0, 0);
}

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
	pDemo->privCreateBuffers();

	// initialize motion blur
	pDemo->motionBlur.initialize(200);
	pDemo->motionBlur.setBlurTime(0.5f);

	// Set up camera
	pDemo->cam.setViewport(0, 0, GAME_WIDTH, GAME_HEIGHT);
	pDemo->cam.setPerspective(35.0f, float(GAME_WIDTH) / float(GAME_HEIGHT), 1.0f, 4500.0f);
	pDemo->cam.setOrientAndPosition(Vect(0.0f, 1.0f, 0.0f), Vect(0.0f, 50.0f, -10.0f), Vect(0.0f, 50.0f, 0.0f));
	pDemo->cam.updateCamera();
	pDemo->projection = pDemo->cam.getProjMatrix();
	pDemo->projection.T();
	pDemo->deviceCon->UpdateSubresource(pDemo->projectionBuffer, 0, nullptr, &pDemo->projection, 0, 0);

	// Set up Lighting
	Vect lightDir(20.0f, 100.0f, 100.0f);
	lightDir.norm();
	Vect lightColor(1.0f, 1.0f, 1.0f, 1.0f);
	pDemo->lightInfo.color = lightColor;
	pDemo->lightInfo.direction = lightDir;
	pDemo->globalLightDir = lightDir;
	pDemo->deviceCon->UpdateSubresource(pDemo->lightBuffer, 0, nullptr, &pDemo->lightInfo, 0, 0);
};

void Demo::Shutdown()
{
	Demo* p = Demo::privGetInstance();

	// Destroy everything
	p->motionBlur.destroy();
	p->privDestroyBuffers();
	p->privDestroyShader();
	p->privShutDownDevice();
}

// Update the demo
void Demo::Update()
{
	Demo* pDemo = Demo::privGetInstance();
	UNUSED(pDemo);

	// Need to keep track of elapsed time
	static clock_t currTime;
	currTime = clock();
	static clock_t lastTime = currTime;
	float elapsedTime = (float) (currTime - lastTime) / CLOCKS_PER_SEC;
	elapsedTime;
	lastTime = currTime;

	// Max frame time
	if (elapsedTime >= 0.05f) elapsedTime = 0.05f;

	// First check to see if time is slowed
	pDemo->privCheckSlowTime(elapsedTime);

	// Move at one tenth speed if so
	if (pDemo->timeSlowed)
	{
		elapsedTime *= 0.10f;
	}

	// Adjust crosshair position
	pDemo->privMoveCrosshairs(elapsedTime);
	
	// Fire bullet
	pDemo->privFireBullet(elapsedTime);

	// update our bullet
	pDemo->bullet.Update(elapsedTime);

	// update our bricks
	for (int i = 0; i < NUM_BRICKS; i++)
	{
		pDemo->bricks[i].Update(elapsedTime);
	}

	// Check for any collisions and handle them
	pDemo->privCheckCollisions(elapsedTime);

	// Check if space bar is pressed 
	// If so reset the demo
	short space = GetKeyState(0x20);
	bool spacePressed = (space & 0x80) != 0;

	if (spacePressed) pDemo->privReset();
};

// Draw our demo
void Demo::Draw()
{
	Demo* pDemo = Demo::privGetInstance();
	UNUSED(pDemo);

	// Clear background for our back buffer and the render texture of motion blur
	float color[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	pDemo->deviceCon->ClearRenderTargetView(pDemo->motionBlur.mainRenderView, (const float*)&color[0]);
	pDemo->deviceCon->ClearRenderTargetView(pDemo->backBufferView, (const float*)&color[0]);

	// Clear the depth buffer to 1.0 (max depth) (for ours and motion blur)
	pDemo->deviceCon->ClearDepthStencilView(pDemo->motionBlur.depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	pDemo->deviceCon->ClearDepthStencilView(pDemo->depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set our render target - we draw everything to motion blur's render target
	pDemo->deviceCon->OMSetRenderTargets(1, &pDemo->motionBlur.mainRenderView, pDemo->motionBlur.depthView);

	// Set our shaders as active
	pDemo->deviceCon->VSSetShader(pDemo->vShader, nullptr, 0);
	pDemo->deviceCon->PSSetShader(pDemo->pShader, nullptr, 0);

	// Set our constant buffers
	pDemo->deviceCon->VSSetConstantBuffers(0, 1, &pDemo->modelViewBuffer);
	pDemo->deviceCon->VSSetConstantBuffers(1, 1, &pDemo->projectionBuffer);
	pDemo->deviceCon->VSSetConstantBuffers(2, 1, &pDemo->lightBuffer);
	pDemo->deviceCon->PSSetConstantBuffers(3, 1, &pDemo->colorBuffer);

	// Set vert buffer
	UINT stride = 6 * sizeof(float);
	UINT offset = 0;
	pDemo->deviceCon->IASetVertexBuffers(0, 1, &pDemo->vertBuffer, &stride, &offset);

	// Set index buffer
	pDemo->deviceCon->IASetIndexBuffer(pDemo->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set input layout
	pDemo->deviceCon->IASetInputLayout(pDemo->inputLayout);

	// Since we're only passing modelview matrix to shader, need to put our light direction
	// into view coordinates
	Matrix rotView = pDemo->cam.getViewMatrix();
	rotView[12] = rotView[13] = rotView[14] = 0.0f;
	pDemo->lightInfo.direction = pDemo->globalLightDir * rotView;
	pDemo->deviceCon->UpdateSubresource(pDemo->lightBuffer, 0, nullptr, &pDemo->lightInfo, 0, 0);

	// Update projection matrix
	// Shouldn't be necessary anymore since camera not moving, but I'll leave it
	Matrix proj = pDemo->cam.getProjMatrix();
	pDemo->deviceCon->UpdateSubresource(pDemo->projectionBuffer, 0, nullptr, &proj, 0, 0);

	// Draw the ground
	pDemo->ground.Draw();

	// Draw our bricks
	for (int i = 0; i < NUM_BRICKS; i++)
	{
		pDemo->bricks[i].Draw();
	}

	// Draw the bullet
	pDemo->bullet.Draw();

	// Now we're making sure our crosshairs are always fully lit.
	// This allows us to get away without writing another shader
	pDemo->lightInfo.direction = Vect(0.0f, 0.0f, 1.0f);
	pDemo->deviceCon->UpdateSubresource(pDemo->lightBuffer, 0, nullptr, &pDemo->lightInfo, 0, 0);
	
	// Don't need projection matrix for drawing crosshairs
	proj.setIdentity();
	pDemo->deviceCon->UpdateSubresource(pDemo->projectionBuffer, 0, nullptr, &proj, 0, 0);
	
	// Draw crosshairs
	pDemo->crosshairX.Draw();
	pDemo->crosshairY.Draw();

	// Now call motionblur.draw, which will draw a combination of frames to the back buffer
	pDemo->motionBlur.draw();

	// Present
	pDemo->swapChain->Present(0, 0);
};

// Run the demo
void Demo::Run()
{
	// Get the instance
	Demo *p = Demo::privGetInstance();

	p->running = true;

	p->privReset();

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
		//SetCapture(Demo::GetWindow());
		break;

	case WM_LBUTTONUP:
		//ReleaseCapture();
		break;

	case WM_RBUTTONDOWN:
		SetCapture(Demo::GetWindow());
		break;

	case WM_RBUTTONUP:
		//SetCapture(Demo::GetWindow());
		ReleaseCapture();
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void Demo::privInitDevice()
{
	UINT sampleCount = 4;
	UINT sampleQuality = 0;

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                  
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  
	scd.BufferDesc.Width = GAME_WIDTH;                  
	scd.BufferDesc.Height = GAME_HEIGHT;               
	//scd.BufferDesc.RefreshRate.Numerator = 60;
	//scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = this->window;                 
	scd.SampleDesc.Count = sampleCount;                       
	scd.SampleDesc.Quality = sampleQuality;                    
	scd.Windowed = TRUE;                          
	//scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


	// create a device, device context and swap chain using the information in the scd struct
	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
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
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (result == E_FAIL)
	{
		printf("Grabbing back buffer failed...\n");
		assert(0);
	}

	// use the back buffer address to create the render target
	result = device->CreateRenderTargetView(backBuffer, NULL, &backBufferView);
	if (result == E_FAIL)
	{
		printf("Creating render target view failed...\n");
		assert(0);
	}
	//pBackBuffer->Release();

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = GAME_WIDTH;
	descDepth.Height = GAME_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = sampleCount;
	descDepth.SampleDesc.Quality = sampleQuality;
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
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	//descDSV.Texture2D.MipSlice = 0;
	result = device->CreateDepthStencilView(depthTexture, &descDSV, &depthView);
	if (FAILED(result))
	{
		printf("Creating depth view failed...\n");
		assert(0);
	}

	// Set the render targets (back buffer of our swap chain and the depth texture we created)
	deviceCon->OMSetRenderTargets(1, &backBufferView, depthView);

	deviceCon->OMSetDepthStencilState(0, 0);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = GAME_WIDTH;
	viewport.Height = GAME_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	deviceCon->RSSetViewports(1, &viewport);

	// Create our sampler state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = device->CreateSamplerState(&sampDesc, &sampler);
	if (FAILED(hr))
	{
		printf("Default sampler creation failed...\n");
		assert(0);
	}

	// Set sampler state as active
	deviceCon->PSSetSamplers(0, 1, &sampler);

	// Hide the cursor
	ShowCursor(false);
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
	if (backBuffer) backBuffer->Release(); backBuffer = 0;
	if (depthView) depthView->Release(); depthView = 0;
	if (depthTexture) depthTexture->Release(); depthTexture = 0;
	if (swapChain) swapChain->Release(); swapChain = 0;
	if (sampler) sampler->Release(); sampler = 0;
	if (device) device->Release(); device = 0;
	if (deviceCon) deviceCon->Release(); deviceCon = 0;
};

// Create our shaders
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
	//descRast.FrontCounterClockwise = true;
	descRast.FrontCounterClockwise = false;
	descRast.DepthClipEnable = true;
	descRast.MultisampleEnable = true;

	hr = this->device->CreateRasterizerState(&descRast, &rastState);
	if (FAILED(hr))
	{
		printf("Rasterizer state creation failed...\n");
		assert(0);
	}

	// Now set all of these as active
	deviceCon->VSSetShader(vShader, nullptr, 0);
	deviceCon->PSSetShader(pShader, nullptr, 0);
	deviceCon->IASetInputLayout(inputLayout);
	deviceCon->RSSetState(rastState);
	deviceCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
};

// Destroy our shaders and associated data
void Demo::privDestroyShader()
{
	if (rastState != 0) { rastState->Release(); rastState = 0; }
	if (inputLayout != 0) { inputLayout->Release(); inputLayout = 0; }
	if (vShader != 0) { vShader->Release(); vShader = 0; }
	if (pShader != 0) { pShader->Release(); pShader = 0; }
}

// Create the necessary buffers
// Constant buffers needed for shaders (matrices/lighting info)
// And vertex and index buffers for our blocks
void Demo::privCreateBuffers()
{
	// CONSTANT BUFFERS ------------------------------------------------------------
	// -----------------------------------------------------------------------------

	// Constant buffer description
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(LightStruct);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	// Light buffer
	HRESULT hr = this->device->CreateBuffer(&bd, nullptr, &lightBuffer);
	if (FAILED(hr))
	{
		printf("Light buffer creation failed...\n");
		assert(0);
	}
	this->deviceCon->UpdateSubresource(lightBuffer, 0, nullptr, &lightInfo, 0, 0);

	// ModelView buffer
	bd.ByteWidth = sizeof(Matrix);
	hr = this->device->CreateBuffer(&bd, nullptr, &modelViewBuffer);
	if (FAILED(hr))
	{
		printf("Model view buffer creation failed...\n");
		assert(0);
	}
	this->deviceCon->UpdateSubresource(modelViewBuffer, 0, nullptr, &modelView, 0, 0);

	// Projection buffer
	bd.ByteWidth = sizeof(Matrix);
	hr = this->device->CreateBuffer(&bd, nullptr, &projectionBuffer);
	if (FAILED(hr))
	{
		printf("Projection buffer creation failed...\n");
		assert(0);
	}
	this->deviceCon->UpdateSubresource(projectionBuffer, 0, nullptr, &projection, 0, 0);

	// Color buffer
	bd.ByteWidth = sizeof(Vect);
	hr = this->device->CreateBuffer(&bd, nullptr, &colorBuffer);
	if (FAILED(hr))
	{
		printf("Color constant buffer creation failed...\n");
		assert(0);
	}
	this->deviceCon->UpdateSubresource(colorBuffer, 0, nullptr, &color, 0, 0);

	// Set our constant buffers
	this->deviceCon->VSSetConstantBuffers(0, 1, &this->modelViewBuffer);
	this->deviceCon->VSSetConstantBuffers(1, 1, &this->projectionBuffer);
	this->deviceCon->VSSetConstantBuffers(2, 1, &this->lightBuffer);
	this->deviceCon->PSSetConstantBuffers(3, 1, &this->colorBuffer);


	// VERT AND INDEX BUFFERS ------------------------------------------------------
	// -----------------------------------------------------------------------------

	// Simple cube mesh I created by hand
	// with normals for lighting
	struct Triangle
	{
		unsigned int vx;
		unsigned int vy;
		unsigned int vz;
	};

	struct Vertex
	{
		float x;
		float y;
		float z;
		float nx;
		float ny;
		float nz;
	};

	// Create the data for our cube
	Triangle triList[12];
	Vertex vertList[24];

	// Left face
	vertList[0].x = -0.5f;
	vertList[0].y = 0.5f;
	vertList[0].z = 0.5f;
	vertList[0].nx = -1.0f;
	vertList[0].ny = 0.0f;
	vertList[0].nz = 0.0f;

	vertList[1].x = -0.5f;
	vertList[1].y = 0.5f;
	vertList[1].z = -0.5f;
	vertList[1].nx = -1.0f;
	vertList[1].ny = 0.0f;
	vertList[1].nz = 0.0f;

	vertList[2].x = -0.5f;
	vertList[2].y = -0.5f;
	vertList[2].z = 0.5f;
	vertList[2].nx = -1.0f;
	vertList[2].ny = 0.0f;
	vertList[2].nz = 0.0f;

	vertList[3].x = -0.5f;
	vertList[3].y = -0.5f;
	vertList[3].z = -0.5f;
	vertList[3].nx = -1.0f;
	vertList[3].ny = 0.0f;
	vertList[3].nz = 0.0f;

	// Right face
	vertList[4].x = 0.5f;
	vertList[4].y = 0.5f;
	vertList[4].z = 0.5f;
	vertList[4].nx = 1.0f;
	vertList[4].ny = 0.0f;
	vertList[4].nz = 0.0f;

	vertList[5].x = 0.5f;
	vertList[5].y = 0.5f;
	vertList[5].z = -0.5f;
	vertList[5].nx = 1.0f;
	vertList[5].ny = 0.0f;
	vertList[5].nz = 0.0f;

	vertList[6].x = 0.5f;
	vertList[6].y = -0.5f;
	vertList[6].z = 0.5f;
	vertList[6].nx = 1.0f;
	vertList[6].ny = 0.0f;
	vertList[6].nz = 0.0f;

	vertList[7].x = 0.5f;
	vertList[7].y = -0.5f;
	vertList[7].z = -0.5f;
	vertList[7].nx = 1.0f;
	vertList[7].ny = 0.0f;
	vertList[7].nz = 0.0f;

	// Front face
	vertList[8].x = -0.5f;
	vertList[8].y = 0.5f;
	vertList[8].z = 0.5f;
	vertList[8].nx = 0.0f;
	vertList[8].ny = 0.0f;
	vertList[8].nz = 1.0f;

	vertList[9].x = 0.5f;
	vertList[9].y = 0.5f;
	vertList[9].z = 0.5f;
	vertList[9].nx = 0.0f;
	vertList[9].ny = 0.0f;
	vertList[9].nz = 1.0f;

	vertList[10].x = -0.5f;
	vertList[10].y = -0.5f;
	vertList[10].z = 0.5f;
	vertList[10].nx = 0.0f;
	vertList[10].ny = 0.0f;
	vertList[10].nz = 1.0f;

	vertList[11].x = 0.5f;
	vertList[11].y = -0.5f;
	vertList[11].z = 0.5f;
	vertList[11].nx = 0.0f;
	vertList[11].ny = 0.0f;
	vertList[11].nz = 1.0f;

	// Back face
	vertList[12].x = -0.5f;
	vertList[12].y = 0.5f;
	vertList[12].z = -0.5f;
	vertList[12].nx = 0.0f;
	vertList[12].ny = 0.0f;
	vertList[12].nz = -1.0f;

	vertList[13].x = 0.5f;
	vertList[13].y = 0.5f;
	vertList[13].z = -0.5f;
	vertList[13].nx = 0.0f;
	vertList[13].ny = 0.0f;
	vertList[13].nz = -1.0f;

	vertList[14].x = -0.5f;
	vertList[14].y = -0.5f;
	vertList[14].z = -0.5f;
	vertList[14].nx = 0.0f;
	vertList[14].ny = 0.0f;
	vertList[14].nz = -1.0f;

	vertList[15].x = 0.5f;
	vertList[15].y = -0.5f;
	vertList[15].z = -0.5f;
	vertList[15].nx = 0.0f;
	vertList[15].ny = 0.0f;
	vertList[15].nz = -1.0f;

	// Top face
	vertList[16].x = -0.5f;
	vertList[16].y = 0.5f;
	vertList[16].z = -0.5f;
	vertList[16].nx = 0.0f;
	vertList[16].ny = 1.0f;
	vertList[16].nz = 0.0f;

	vertList[17].x = 0.5f;
	vertList[17].y = 0.5f;
	vertList[17].z = -0.5f;
	vertList[17].nx = 0.0f;
	vertList[17].ny = 1.0f;
	vertList[17].nz = 0.0f;

	vertList[18].x = -0.5f;
	vertList[18].y = 0.5f;
	vertList[18].z = 0.5f;
	vertList[18].nx = 0.0f;
	vertList[18].ny = 1.0f;
	vertList[18].nz = 0.0f;

	vertList[19].x = 0.5f;
	vertList[19].y = 0.5f;
	vertList[19].z = 0.5f;
	vertList[19].nx = 0.0f;
	vertList[19].ny = 1.0f;
	vertList[19].nz = 0.0f;

	// Bottom face
	vertList[20].x = -0.5f;
	vertList[20].y = -0.5f;
	vertList[20].z = -0.5f;
	vertList[20].nx = 0.0f;
	vertList[20].ny = -1.0f;
	vertList[20].nz = 0.0f;

	vertList[21].x = 0.5f;
	vertList[21].y = -0.5f;
	vertList[21].z = -0.5f;
	vertList[21].nx = 0.0f;
	vertList[21].ny = -1.0f;
	vertList[21].nz = 0.0f;

	vertList[22].x = -0.5f;
	vertList[22].y = -0.5f;
	vertList[22].z = 0.5f;
	vertList[22].nx = 0.0f;
	vertList[22].ny = -1.0f;
	vertList[22].nz = 0.0f;

	vertList[23].x = 0.5f;
	vertList[23].y = -0.5f;
	vertList[23].z = 0.5f;
	vertList[23].nx = 0.0f;
	vertList[23].ny = -1.0f;
	vertList[23].nz = 0.0f;

	// Triangle lists
	triList[0].vx = 1;
	triList[0].vy = 0;
	triList[0].vz = 3;

	triList[1].vx = 3;
	triList[1].vy = 0;
	triList[1].vz = 2;

	triList[2].vx = 6;
	triList[2].vy = 4;
	triList[2].vz = 5;

	triList[3].vx = 6;
	triList[3].vy = 5;
	triList[3].vz = 7;

	triList[4].vx = 10;
	triList[4].vy = 8;
	triList[4].vz = 9;

	triList[5].vx = 10;
	triList[5].vy = 9;
	triList[5].vz = 11;

	triList[6].vx = 13;
	triList[6].vy = 12;
	triList[6].vz = 15;

	triList[7].vx = 15;
	triList[7].vy = 12;
	triList[7].vz = 14;

	triList[8].vx = 16;
	triList[8].vy = 17;
	triList[8].vz = 18;

	triList[9].vx = 18;
	triList[9].vy = 17;
	triList[9].vz = 19;

	triList[10].vx = 23;
	triList[10].vy = 21;
	triList[10].vz = 20;

	triList[11].vx = 23;
	triList[11].vy = 20;
	triList[11].vz = 22;

	// Now create vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &vertList[0];
	hr = this->device->CreateBuffer(&bd, &InitData, &vertBuffer);
	if (FAILED(hr))
	{
		printf("Vertex buffer creation failed...\n");
		assert(0);
	}

	// Now create index buffer
	bd.ByteWidth = sizeof(Triangle) * 12;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &triList[0];
	hr = this->device->CreateBuffer(&bd, &InitData, &indexBuffer);
	if (FAILED(hr))
	{
		printf("Index buffer creation failed...\n");
		assert(0);
	}

	// Set vert buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deviceCon->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);

	// Set index buffer
	deviceCon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
};

// Destroy the buffers we created
void Demo::privDestroyBuffers()
{
	// Delete constant buffers
	if (modelViewBuffer) { modelViewBuffer->Release(); modelViewBuffer = 0; }
	if (projectionBuffer) { projectionBuffer->Release(); projectionBuffer = 0; }
	if (lightBuffer) { lightBuffer->Release(); lightBuffer = 0; }
	if (colorBuffer) { colorBuffer->Release(); colorBuffer = 0; }

	// Delete vert and index buffers
	if (vertBuffer) { vertBuffer->Release(); vertBuffer = 0; }
	if (indexBuffer) { indexBuffer->Release(); indexBuffer = 0; }
};
