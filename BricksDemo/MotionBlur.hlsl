//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer ConstBufferBlur : register(b0)
{
	int startIndex;
	int numUsed;
	int numTextures;
	float percentageEach;
};

// Texures/samplers
Texture2DArray Textures : register(t0);
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
	// Combine all of our past frames with the current one
	float4 pixelColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 texCoord = float3(input.Tex.x, input.Tex.y, 0.0f);

	int currIndex = startIndex;

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
