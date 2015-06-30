//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

// Info about which of our past frames we should blur together
cbuffer ConstBufferBlur : register(b0)
{
	int startIndex;
	int numUsed;
	int numTextures;
	float percentageEach;
};

// Array of textures that are our past frames
Texture2DArray Textures : register(t0);

// Generic sampler
SamplerState Sampler0 : register(s0);

//--------------------------------------------------------------------------------------
// Shader Input
struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VShader( VS_INPUT input ) 
{
	// No view or projection matrices needed
	// The buffer holds 2 triangles covering the screen
    PS_INPUT output = (PS_INPUT)0;
	output.Pos = input.Pos;
	output.Pos.w = 1.0f;
	output.Tex = input.Tex;
    return output;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PShader( PS_INPUT input) : SV_Target
{
	float4 pixelColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 texCoord = float3(input.Tex.x, input.Tex.y, 0.0f);

	int currIndex = startIndex;

	// Combine all of the indicated past frames equally
	for (int i = 0; i < numUsed; i++)
	{
		texCoord.z = float(currIndex);
		pixelColor += percentageEach * Textures.Sample(Sampler0, texCoord);

		currIndex++;
		if (currIndex >= numTextures)
		{
			currIndex = 0;
		}
	} 

	pixelColor.a = 1.0f;

	return pixelColor;
};
