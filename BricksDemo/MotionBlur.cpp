#include "MotionBlur.h"
#include "Demo.h"
#include <stdio.h>
#include <cmath>
#include <chrono>

// Default constructor
MotionBlur::MotionBlur()
	: blurTextures(0),
	blurSRVs(0),
	depthTexture(0),
	depthView(0),
	currRTV(0),
	vertBuffer(0),
	indexBuffer(0),
	vShader(0),
	pShader(0),
	inputLayout(0),
	blurTime(0.5f),
	lastTime(),
	currIndex(0),
	numTextures(0),
	initCount(0),
	blurOn(0)
{
};

// Initialize the object
// Create all of our textures and buffers (quite a lot)
void MotionBlur::initialize(const int numTexturesIn)
{
	this->numTextures = numTexturesIn;
	cbBlur.numTextures = numTexturesIn;

	int sampleCount = 4, sampleQuality = 0;

	// Now create our main texture that we will render to, then 
	// Resolve the multisampling into our blur textures
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = GAME_WIDTH;
	textureDesc.Height = GAME_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = sampleCount;
	textureDesc.SampleDesc.Quality = sampleQuality;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	HRESULT hr;
	ID3D11Device* dev = Demo::GetDevice();

	// Create the main render target texture
	hr = dev->CreateTexture2D(&textureDesc, nullptr, &mainRenderTexture);
	if (hr != S_OK)
	{
		printf("Texture creation for motion blur failed...\n");
		assert(0);
	}

	// Create the main render target View
	hr = dev->CreateRenderTargetView(mainRenderTexture, nullptr, &mainRenderView);
	if (FAILED(hr))
	{
		printf("Creating render target view for motion blur failed...\n");
		assert(0);
	}

	// Create the texture array for storing past frames
	textureDesc.ArraySize = numTextures;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = numTextures;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;

	// Here is texture creation
	hr = dev->CreateTexture2D(&textureDesc, nullptr, &blurTextures);
	if (hr != S_OK)
	{
		printf("Texture creation for motion blur failed...\n");
		assert(0);
	}

	// And shader resource view creation
	hr = dev->CreateShaderResourceView(blurTextures, &srvDesc, &blurSRVs);
	if (FAILED(hr))
	{
		printf("Shader resource view creation for motion blur failed...\n");
		assert(0);
	}

	// Now create the depth buffer
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.SampleDesc.Count = sampleCount;
	textureDesc.SampleDesc.Quality = sampleQuality;

	hr = dev->CreateTexture2D(&textureDesc, nullptr, &depthTexture);
	if (FAILED(hr))
	{
		printf("Creating depth texture for motion blur failed...\n");
		assert(0);
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = textureDesc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	hr = dev->CreateDepthStencilView(depthTexture, &descDSV, &depthView);
	if (FAILED(hr))
	{
		printf("Creating depth view for motion blur failed...\n");
		assert(0);
	}

	// VERTEX AND INDEX INFO
	// Creating a simple vertex and index buffer for drawing a rectangle filling the screen
	// Just two triangles
	struct Triangle
	{
		unsigned int v1;
		unsigned int v2;
		unsigned int v3;
	};

	struct Vertex
	{
		float x;
		float y;
		float z;
		float s;
		float t;
	};

	Vertex verts[4];
	memset(&verts[0], 0, sizeof(Vertex) * 4);

	verts[0].x = -1.0f;
	verts[0].y = 1.0f;
	verts[0].z = 0.5f;
	verts[0].s = 0.0f;
	verts[0].t = 0.0f;

	verts[1].x = 1.0f;
	verts[1].y = 1.0f;
	verts[1].z = 0.5f;
	verts[1].s = 1.0f;
	verts[1].t = 0.0f;

	verts[2].x = 1.0f;
	verts[2].y = -1.0f;
	verts[2].z = 0.5f;
	verts[2].s = 1.0f;
	verts[2].t = 1.0f;

	verts[3].x = -1.0f;
	verts[3].y = -1.0f;
	verts[3].z = 0.5f;
	verts[3].s = 0.0f;
	verts[3].t = 1.0f;

	// Triangle list
	Triangle tList[2];

	tList[0].v1 = 1;
	tList[0].v2 = 2;
	tList[0].v3 = 3;

	tList[1].v1 = 0;
	tList[1].v2 = 1;
	tList[1].v3 = 3;

	// Here actually create vertex and index buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &verts[0];
	hr = dev->CreateBuffer(&bd, &InitData, &vertBuffer);
	if (FAILED(hr))
	{
		printf("Vertex buffer creation for motion blur failed...\n");
		assert(0);
	}

	bd.ByteWidth = sizeof(Triangle) * 2;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &tList[0];
	hr = Demo::GetDevice()->CreateBuffer(&bd, &InitData, &indexBuffer);
	if (FAILED(hr))
	{
		printf("Index buffer creation failed...\n");
		assert(0);
	}

	// Now need to make constant buffer
	// Constant buffer description
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBBlur);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	// Create the buffer
	hr = Demo::GetDevice()->CreateBuffer(&bd, nullptr, &blurInfoBuffer);
	if (FAILED(hr))
	{
		printf("Motion blur info constant buffer creation failed...\n");
		assert(0);
	}
	Demo::GetDeviceContext()->UpdateSubresource(blurInfoBuffer, 0, nullptr, &cbBlur, 0, 0);


	// SHADERS ----------------------------------------------
	// Now create our shaders for motion blur effect
	ID3DBlob* vShaderBlob = 0;
	ID3DBlob* pShaderBlob = 0;

	hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Skip optimization and add debug info if we're in debug configuration
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// Create the vertex shader
	ID3DBlob* pErrorBlob = nullptr;

	// Compile vertex shader
	hr = D3DCompileFromFile(L"MotionBlur.hlsl", nullptr, nullptr, "VShader", "vs_4_0",
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
	hr = Demo::GetDevice()->CreateVertexShader(vShaderBlob->GetBufferPointer(), vShaderBlob->GetBufferSize(), nullptr, &vShader);
	if (FAILED(hr))
	{
		printf("Vertex shader creation failed.\n");
		vShaderBlob->Release();
		assert(0);
	}

	pErrorBlob = 0;
	// Compile pixel shader
	hr = D3DCompileFromFile(L"MotionBlur.hlsl", nullptr, nullptr, "PShader", "ps_4_0",
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
	hr = Demo::GetDevice()->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &pShader);
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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	hr = Demo::GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), vShaderBlob->GetBufferPointer(),
		vShaderBlob->GetBufferSize(), &inputLayout);

	// Release our blobs
	vShaderBlob->Release();
	pShaderBlob->Release();

	if (FAILED(hr))
	{
		printf("Input layout creation failed...\n");
		assert(0);
	}

	lastTime = clock();
};

// Destroy all our created objects
void MotionBlur::destroy()
{
	// Delete all of our rendering textures
	if (blurTextures != 0) blurTextures->Release(); blurTextures = 0;
	if (blurSRVs != 0) blurSRVs->Release(); blurSRVs = 0;

	// Delete our shaders
	if (vShader != 0) vShader->Release(); vShader = 0;
	if (pShader != 0) pShader->Release(); pShader = 0;
	if (inputLayout != 0) inputLayout->Release(); inputLayout = 0;

	// Delete everything else
	if (mainRenderTexture != 0) mainRenderTexture->Release(); mainRenderTexture = 0;
	if (mainRenderView != 0) mainRenderView->Release(); mainRenderView = 0;
	if (depthTexture != 0) depthTexture->Release(); depthTexture = 0;
	if (depthView != 0) depthView->Release(); depthView = 0;
	if (currRTV != 0) currRTV->Release(); currRTV = 0;
	if (vertBuffer != 0) vertBuffer->Release(); vertBuffer = 0;
	if (indexBuffer != 0) indexBuffer->Release(); indexBuffer = 0;
};

// Destructor
MotionBlur::~MotionBlur()
{
	ZeroMemory(this, sizeof(MotionBlur));
};

// Init count keeps track to make sure we don't use more 
// past frames than we've actually drawn
void MotionBlur::resetInitCount()
{
	this->initCount = 0;
};

// Draw
void MotionBlur::draw()
{
	clock_t currTime;
	currTime = clock();
	float elapsedTime = float(currTime - lastTime) / float(CLOCKS_PER_SEC);
	lastTime = currTime;

	// To prevent jitter, we need to average last frametimes
	// This gets rid of instantaneous variation in amount of blur
	static float times[20];
	static int counter = 0;
	times[counter] = elapsedTime;
	counter++;
	if (counter >= 20) counter = 0;
	float timeTotal = 0.0f;
	for (int j = 0; j < 20; j++)
	{
		timeTotal = timeTotal + times[j];
	}
	float timeToUse = timeTotal / 20.0f;

	// Make sure we're not using more frames than we've drawn yet
	initCount++;
	if (initCount > numTextures)
	{
		initCount = numTextures;
	}

	// Update blur info for shader
	// Calculate how many of our frames we should use
	int numFramesToUse = (int) floor(blurTime / timeToUse + 0.5f);
	if (numFramesToUse <= 0) numFramesToUse = 1;
	if (numFramesToUse > initCount)
	{
		numFramesToUse = initCount;
	}

	if (numFramesToUse >= numTextures)
	{
		numFramesToUse = numTextures;
		cbBlur.startIndex = 0;
	}
	else
	{
		cbBlur.startIndex = currIndex;
	}

	// Set rest of data in our constant buffer
	cbBlur.percentageEach = 1.0f / float(numFramesToUse);
	cbBlur.numUsed = numFramesToUse;
	cbBlur.numTextures = numTextures;

	// If blur is off, we are still using this shader
	// Just using the current frame, with a percentage of 1.0
	if (!blurOn)
	{
		cbBlur.startIndex = currIndex;
		cbBlur.numUsed = 1;
		cbBlur.percentageEach = 1.0f;
	}

	ID3D11DeviceContext* devCon = Demo::GetDeviceContext();

	ID3D11Texture2D* backBuffer = Demo::GetBackBuffer();
	ID3D11RenderTargetView* backBufferView = Demo::GetBackBufferView();
	ID3D11DepthStencilView* mainDepthView = Demo::GetDepthView();

	// Demo::draw already draw the current screen to our main render texture here

	// Now render to screen, using blur shader
	devCon->OMSetRenderTargets(1, &backBufferView, mainDepthView);

	// Set our shaders as active
	devCon->VSSetShader(vShader, nullptr, 0);
	devCon->PSSetShader(pShader, nullptr, 0);
	// Set input layout
	devCon->IASetInputLayout(inputLayout);

	// First need to resolve our multi sampled drawing
	// Make sure it's not bound anywhere
	ID3D11ShaderResourceView* nullView = 0;
	devCon->PSSetShaderResources(0, 1, &nullView);
	devCon->ResolveSubresource(blurTextures, currIndex, mainRenderTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	// Make sure right textures are bound
	devCon->PSSetShaderResources(0, 1, &blurSRVs);

	// Update our constant buffer
	devCon->UpdateSubresource(blurInfoBuffer, 0, nullptr, &cbBlur, 0, 0);
	devCon->PSSetConstantBuffers(0, 1, &blurInfoBuffer);

	// Set vert buffer
	UINT stride = sizeof(float) * 5;
	UINT offset = 0;
	devCon->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);

	// Set index buffer
	devCon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Actual draw call
	devCon->DrawIndexed(6, 0, 0);

	// Update the index
	currIndex--;
	if (currIndex < 0)
	{
		currIndex = numTextures - 1;
	}
};

// Set total time into past that should be blurred
void MotionBlur::setBlurTime(const float timeIn)
{
	this->blurTime = timeIn;
}
