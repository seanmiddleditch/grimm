#if !defined(_COMMON_SHADER_)
#define _COMMON_SHADER_

//--------------------------------------------------------------------------------------
// Constant Definitions
//--------------------------------------------------------------------------------------

static const float _EPSILON = 1e-6f; // 0.000001f

static const float _PI	= 3.141592654;
static const float _2PI = 6.283185307;

//--------------------------------------------------------------------------------------
// Sampler States. Registers must match those in RenderMaterial.cpp
//--------------------------------------------------------------------------------------

// Point Samplers
SamplerState SS_POINT_CLAMP		: register(s0);
SamplerState SS_POINT_WRAP		: register(s1);

// Bilinear Samplers
SamplerState SS_BILINEAR_CLAMP	: register(s2);
SamplerState SS_BILINEAR_WRAP	: register(s3);

// Trilinear Samplers
SamplerState SS_TRILINEAR_CLAMP	: register(s4);
SamplerState SS_TRILINEAR_WRAP	: register(s5);

// Anisotropic Samplers
SamplerState SS_ANISO_CLAMP		: register(s6);
SamplerState SS_ANISO_WRAP		: register(s7);

//--------------------------------------------------------------------------------------
// Common Helper functions
//--------------------------------------------------------------------------------------

float pow5(float x)
{
	float x2 = x * x;
	float x4 = x2 * x2;
	return x4 * x;
}

// https://en.wikipedia.org/wiki/Relative_luminance
float LinearToRelativeLuminance(float3 color)
{
	return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

// Linear <--> Gamma Conversions
// https://en.wikipedia.org/wiki/SRGB
float3 LinearToGamma(float3 color)
{
	return color < 0.0031308 ? 12.92 * color : 1.055 * pow(color, 1.0 / 2.4) - 0.055;
}

float4 LinearToGamma(float4 color)
{
	color.rgb = LinearToGamma(color.rgb);
	return color;
}

float3 GammaToLinear(float3 color)
{
	return color < 0.04045 ? color / 12.92 : pow((color + 0.055) / 1.055, 2.4);
}

float4 GammaToLinear(float4 color)
{
	color.rgb = GammaToLinear(color.rgb);
	return color;
}

//--------------------------------------------------------------------------------------
// Normal Mapping Helper Functions
//--------------------------------------------------------------------------------------

float3 DecodeNormal(float2 normalSample, float scale = 1.0)
{
	// Decode RGB normal to vector, [0, 1] to [-1, 1]
	float2 txNormal	= normalSample.rg * 2.0 - 1.0;
	float3 TBNNormal = float3(txNormal.x, txNormal.y, 0);
	
	// Calculate Z
	//TBNNormal.z	= sqrt(abs(1.0 - TBNNormal.x * TBNNormal.x - TBNNormal.y * TBNNormal.y));
	TBNNormal.z	= sqrt(1.0 - dot(TBNNormal.xy, TBNNormal.xy));
	
	// Scale and normalize
	TBNNormal.xy *= scale;
	
	return normalize(TBNNormal);
}

float3 TBNormalToWorldNormal(float3 TBNormal, float3 worldNormal, float3 worldTangent, bool forceOrthogonal = true)
{
	float3 N = worldNormal;
	float3 T = worldTangent;
	
	// Ensure TBN is orthoganal using the Gram-Schmidt process 
	T = forceOrthogonal ? normalize(T - dot(T, N) * N) : T;

	float3 B = cross(T, N);
	
	float3x3 TBN = float3x3(T, B, N);

	return mul(TBNormal, TBN);
}

#endif // #if !defined(_COMMON_SHADER_)