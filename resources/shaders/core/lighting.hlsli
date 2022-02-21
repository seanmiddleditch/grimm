//--------------------------------------------------------------------------------------
// Forward lighting equations and utility functions.
// All forward lighting is calculated in world space.
//--------------------------------------------------------------------------------------

#if !defined(_LIGHTING_SHADER_)
#define _LIGHTING_SHADER_

#include "core/common.hlsli"
#include "core/brdf.hlsli"

//--------------------------------------------------------------------------------------
// Calculate Forward Lighting
//--------------------------------------------------------------------------------------

void CalculateForwardLighting(  in float3  viewDirection,
                                in float3  lightDirection,
                                in float3  normal,
                                in float   roughness,
                                in float3  f0,
                                out float3 diffuseLight,
                                out float3 specularLight)
{     
    // Call active BRDF and return outgoing Diffuse and Specular values

    //DisneyPricipledBRDF(viewDirection, lightDirection, normal, roughness, f0, diffuseLight, specularLight);
    FrostbiteBRDF(viewDirection, lightDirection, normal, roughness, f0, diffuseLight, specularLight);
    //UE4BRDF(viewDirection, lightDirection, normal, roughness, f0, diffuseLight, specularLight);
}

//--------------------------------------------------------------------------------------
// IBL Specular Lookups
//--------------------------------------------------------------------------------------

// DFG LUT lookup
float3 EnvDFGLUT(float2 lut, float3 f0)
{
    return f0 * lut.x + lut.y;
}

// Analytical DFG approximations

// Unreal4 DFG Approximation - Best when used with UE4 BRDF
// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
float3 EnvDFGUnreal4( float3 specularColor, float roughness, float NdotV )
{
	const float4 c0 = {-1, -0.0275, -0.572, 0.022};
	const float4 c1 = { 1,  0.0425,  1.04,  -0.04};
	float4 r = roughness * c0 + c1;
	float  a004 = min(r.x * r.x, exp2(-9.28 * NdotV)) * r.x + r.y;
	float2 AB   = float2(-1.04, 1.04) * a004 + r.zw;
	
    return specularColor * AB.x + AB.y;
}

// https://knarkowicz.wordpress.com/2014/12/27/analytical-dfg-term-for-ibl/
float3 EnvDFGLazarov( float3 specularColor, float roughness, float NdotV )
{
    const float4 p0 = float4(0.5745,  1.548,  -0.02397, 1.301);
    const float4 p1 = float4(0.5753, -0.2511, -0.02066, 0.4755);
 
    float  gloss = 1.0 - roughness;
    float4 t = gloss * p0 + p1;
 
    float bias = saturate(t.x * min(t.y, exp2(-7.672 * NdotV)) + t.z);
    float delta = saturate(t.w);
    float scale = delta - bias;
 
    bias *= saturate(50.0 * specularColor.y);
    
    return specularColor * scale + bias;
}

float3 EnvDFGPolynomial( float3 specularColor, float roughness, float NdotV)
{
    float x = 1.0 - roughness; // gloss
    float y = NdotV;
 
    const float b1 = -0.1688;
    const float b2 = 1.895;
    const float b3 = 0.9903;
    const float b4 = -4.853;
    const float b5 = 8.404;
    const float b6 = -5.069;
    float bias = saturate( min( b1 * x + b2 * x * x, b3 + b4 * y + b5 * y * y + b6 * y * y * y ) );
 
    const float d0 = 0.6045;
    const float d1 = 1.699;
    const float d2 = -0.5228;
    const float d3 = -3.603;
    const float d4 = 1.404;
    const float d5 = 0.1939;
    const float d6 = 2.661;
    float delta = saturate( d0 + d1 * x + d2 * y + d3 * x * x + d4 * x * y + d5 * y * y + d6 * x * x * x );
    float scale = delta - bias;
 
    bias *= saturate( 50.0 * specularColor.y );
    
    return specularColor * scale + bias;
}

//--------------------------------------------------------------------------------------
// Occlusion Helper Functions
//--------------------------------------------------------------------------------------

// Horizon Occlusion
// http://marmosetco.tumblr.com/post/81245981087
float SpecularHorizonOcclusion(float3 reflection, float3 vertexNormal, float horizonFade)
{
	float horizon = saturate(1.0 + horizonFade * dot(reflection, vertexNormal));
	horizon *= horizon;

	return horizon;
}

// Irradiance maps are pre-intergrated across the hemisphere, thus containing lighting data
// that sould be occludded by the geometry. Thus... scale indirect diffuse based on the dot
// product of the surface normal and the mesh normal. Not physically accurate!!
float DiffuseHorizonOcclusion(float3 normal, float3 vertexNormal, float horizonFade)
{
	float horizon = 1.0 - dot(normal, vertexNormal);
	horizon = 1.0 - saturate(horizon * horizonFade);
	horizon = (horizon + 1.0) * 0.5; // horizon = lerp(0.5, 1.0, horizon);
	horizon *= horizon;

	return horizon;
}

// Micro-Shadows
// http://advances.realtimerendering.com/other/2016/naughty_dog/NaughtyDog_TechArt_Final.pdf

float MicroShadow(float ao, float3 N, float3 L)
{
    float aperture = 2.0 * ao * ao;
    float microShadow = saturate(abs(dot(L, N)) + aperture - 1.0);

    return microShadow;
}

#endif // #if !defined(_LIGHTING_SHADER_)
