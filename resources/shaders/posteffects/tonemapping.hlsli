//--------------------------------------------------------------------------------------
// Tonemapping Operators
//--------------------------------------------------------------------------------------

#if !defined(_TONEMAPPING_SHADER_)
#define _TONEMAPPING_SHADER_

#include "core/common.hlsli"

// Reinhard
// http://filmicworlds.com/blog/filmic-tonemapping-operators/

float3 Reinhard(float3 color)
{
    return color / (1.0 + color);
}

// Hable Uncharted2
// http://filmicworlds.com/blog/filmic-tonemapping-operators/

float3 Opperator(float3 x)
{
    static const float A = 0.15;
    static const float B = 0.50;
    static const float C = 0.10;
    static const float D = 0.20;
    static const float E = 0.02;
    static const float F = 0.30;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - (E / F);
}

float3 Uncharted2(float3 color)
{
    static const float W = 11.2;
    static const float ExposureBias = 2.0f;

	float3 curr = Opperator(ExposureBias * color);
	float3 whiteScale = 1.0f / Opperator(W);
	
    return color = curr * whiteScale;
}

// ACES Fitted Curve by Stephen Hill
// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786) - 0.000090537;
    float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

#endif // #if !defined(_TONEMAPPING_SHADER_)
