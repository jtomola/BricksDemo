//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

// Buffer holding modelview matrix
cbuffer ConstBufferMV : register(b0)
{
	matrix ModelView;
};

// Buffer holding projection matrix
cbuffer ConstBufferP : register(b1)
{
	matrix Projection;
};

// Buffer holding light info
cbuffer ConstBufferLight : register(b2)
{
	float4 LightDir; // In view coordinates
	float4 LightColor;
};

// Buffer holding color to apply to all vertices
cbuffer ConstBufferColor : register(b3)
{
	float4 Color;
};


//--------------------------------------------------------------------------------------
// Shader Input

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VShader(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, ModelView);

		// Need to transform normal - only rotation in modelview
		float3x3 normalMatrix;
	normalMatrix._m00_m01_m02 = normalize(ModelView._m00_m01_m02);
	normalMatrix._m10_m11_m12 = normalize(ModelView._m10_m11_m12);
	normalMatrix._m20_m21_m22 = normalize(ModelView._m20_m21_m22);
	float3 transNorm = normalize(mul(input.Norm, normalMatrix));

	// Lighting, use dot of normal and light position for all 3
	output.Color = 0;
	output.Color += saturate(dot(LightDir.xyz, transNorm) * LightColor);
	output.Color.a = 1.0f;

	output.Pos = mul(output.Pos, Projection);
	return output;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PShader(PS_INPUT input) : SV_Target
{
	return Color * input.Color;
};