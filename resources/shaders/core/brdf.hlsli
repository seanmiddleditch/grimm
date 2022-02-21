//--------------------------------------------------------------------------------------
// BRDF Lighting equations and utility functions.
//--------------------------------------------------------------------------------------

#if !defined(_BRDF_SHADER_)
#define _BRDF_SHADER_

#include "core/common.hlsli"

//--------------------------------------------------------------------------------------
// D - Normal Distribution Functions (NDF)
//--------------------------------------------------------------------------------------

// GGX - Trowbridge-Reitz
float D_GGX(float NdotH, float alpha)
{
    float alphaSquared = alpha * alpha; 
    float denominator = NdotH * NdotH * (alphaSquared - 1.0) + 1.0;
    //float denominator = (NdotH * alphaSquared - NdotH) * NdotH + 1.0;

    return alphaSquared / (denominator * denominator * _PI);
}

float D_GGXAniso(float NdotH, float HdotX, float HdotY, float alphaX, float alphaY)
{
    float alphaSquaredX = alphaX * alphaX;
    float alphaSquaredY = alphaY * alphaY;
    float denominator = (HdotX * HdotX / alphaSquaredX) + (HdotY * HdotY / alphaSquaredY) + (NdotH * NdotH);

    return 1.0 / (alphaX * alphaY * (denominator * denominator) * _PI);
}

// GTR - Generalized-Trowbridge-Reitz (Disney)
// Secondary lobe with a longer tail than GGX, used for clearcoat layer
float D_GTR1(float NdotH, float alpha)
{
    float alphaSquared = alpha * alpha;
    float denominator1 = log(alphaSquared);
    float denominator2 = (1.0 + (alphaSquared - 1.0) * (NdotH * NdotH));

    return (alphaSquared - 1.0) / (denominator1 * denominator2 * _PI);
}

// Primary lobe used for base layer - equivalent to GGX
float D_GTR2(float NdotH, float alpha)
{
    float alphaSquared = alpha * alpha;
    float denominator = (1.0 + (alphaSquared - 1.0) * (NdotH * NdotH));

    return alphaSquared / (denominator * denominator * _PI);
}

// Equivalent to GGXAniso
float D_GTR2Aniso(float NdotH, float HdotX, float HdotY, float alphaX, float alphaY)
{
    float alphaSquaredX = alphaX * alphaX;
    float alphaSquaredY = alphaY * alphaY;
    float denominator = (HdotX * HdotX / alphaSquaredX) + (HdotY * HdotY / alphaSquaredY) + (NdotH * NdotH);

    return 1.0 / (alphaSquaredX * alphaSquaredY * (denominator * denominator) * _PI);
}

//--------------------------------------------------------------------------------------
// F - Fresnel Terms
//--------------------------------------------------------------------------------------

// Schlick's Fresnel approximation
float3 F_SchlickFresnel(float3 f0, float dotProduct)
{
    return f0 + (1.0 - f0) * pow5(1.0 - dotProduct);
}

// Schlick's Fresnel approximation (Unreal4)
float3 F_SchlickFresnelUE4(float3 f0, float dotProduct)
{
    return f0 + (1.0 - f0) * pow(2.0, (-5.55473 * dotProduct) - (-6.98316 * dotProduct));
}

// Schlick's Fresnel approximation with f90
float3 F_SchlickFresnel(float3 f0, float3 f90, float dotProduct)
{
    return f0 + (f90 - f0) * pow5(1.0 - dotProduct);
}

// Schlick's Fresnel approximation with roughness
float3 F_SchlickFresnelRoughness(float3 f0, float dotProduct, float roughness)
{
    return f0 + (max(1.0 - roughness, f0) - f0) * pow5(1.0 - dotProduct);
} 

//--------------------------------------------------------------------------------------
// G - Geometric Visibility Terms
//--------------------------------------------------------------------------------------

// Implicit
float G_Implicit(float NdotL, float NdotV)
{
    return NdotL * NdotV;
}

// Schlick-Beckmann matched to GGX (Unreal4)
float G_SchlickGGX(float NdotL, float NdotV, float alpha)
{
    float K = alpha * 0.5;
    float G_Schlick_L = NdotL / (NdotL * (1.0 - K) + K);
    float G_Schlick_V = NdotV / (NdotV * (1.0 - K) + K);

    return 0.25 * G_Schlick_L * G_Schlick_V;
}

// Smith GGX Corellated (Frostbite)
float G_SmithGGXCorrelated(float NdotL, float NdotV, float alpha)
{
    float alphaSquared = alpha * alpha;
    float Lambda_GGXV = NdotL * sqrt((NdotV - alphaSquared * NdotV) * NdotV + alphaSquared);
    float Lambda_GGXL = NdotV * sqrt((NdotL - alphaSquared * NdotL) * NdotL + alphaSquared);

    return 0.5 / (Lambda_GGXV + Lambda_GGXL + _EPSILON);
}

//--------------------------------------------------------------------------------------
// Diffuse Terms
//--------------------------------------------------------------------------------------

// Lambert Diffuse
float LambertDiffuse(float NdotL)
{
    return NdotL / _PI;
}

// Disney Diffuse
float DisneyDiffuse(float NdotV,
                    float NdotL,
                    float LdotH,
                    float linearRoughness)
{
    float  f90          = 0.5 + 2.0 * LdotH * LdotH * linearRoughness;
    float3 f0           = float3(1.0, 1.0, 1.0);
    float  lightScatter = F_SchlickFresnel(f0, f90, NdotL).r;
    float  viewScatter  = F_SchlickFresnel(f0, f90, NdotV).r;

    return lightScatter * viewScatter * NdotL / _PI;
}

// Disney Diffuse modified with energy conservation (Frostbite)
float DisneyDiffuseModified(float NdotV,
                            float NdotL,
                            float LdotH,
                            float linearRoughness)
{
    float  energyBias   = lerp(0.0, 0.5, linearRoughness);
    float  energyFactor = lerp(1.0, 1.0 / 1.51, linearRoughness);
    float  f90          = energyBias + 2.0 * LdotH * LdotH * linearRoughness;
    float3 f0           = float3(1.0, 1.0, 1.0);
    float  lightScatter = F_SchlickFresnel(f0, f90, NdotL).r;
    float  viewScatter  = F_SchlickFresnel(f0, f90, NdotV).r;

    return lightScatter * viewScatter * energyFactor * NdotL / _PI;
}

//--------------------------------------------------------------------------------------
// BRDFs
//--------------------------------------------------------------------------------------

// Disney "Pricipled" BRDF
// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
void DisneyPricipledBRDF(   in float3  viewDirection,
                            in float3  lightDirection,
                            in float3  normal,
                            in float   roughness,
                            in float3  f0,
                            out float3 diffuseLight,
                            out float3 specularLight)
{
    float r2 = roughness * roughness; // perceptual linear roughness

    float3 H = normalize(viewDirection + lightDirection);
    float  NdotL = saturate(dot(normal, lightDirection));
	float  NdotV = abs(dot(normal, viewDirection)) + _EPSILON; // avoid artifacts
    float  NdotH = saturate(dot(normal, H));
    float  LdotH = saturate(dot(lightDirection, H));

    float  D    = D_GTR2(NdotH, r2);
    float3 F    = F_SchlickFresnel(f0, LdotH);
    float  G    = G_SmithGGXCorrelated(NdotL, NdotV, r2);

    specularLight   = D * F * G * NdotL * 0.25;
    diffuseLight    = DisneyDiffuse(NdotV, NdotL, LdotH, r2);

    diffuseLight *= (1.0 - specularLight); // Energy conservation - normalize diffuse with inverted specular term
}

// EA Frostbite Modified Disney BRDF - Extended and optimized for real time rendering
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
void FrostbiteBRDF( in float3  viewDirection,
                    in float3  lightDirection,
                    in float3  normal,
                    in float   roughness,
                    in float3  f0,
                    out float3 diffuseLight,
                    out float3 specularLight)
{
    float r2 = roughness * roughness; // perceptual linear roughness

    float3 H = normalize(viewDirection + lightDirection);
    float  NdotL = saturate(dot(normal, lightDirection));
	float  NdotV = abs(dot(normal, viewDirection)) + _EPSILON; // avoid artifacts
    float  NdotH = saturate(dot(normal, H));
    float  LdotH = saturate(dot(lightDirection, H));

    float  D    = D_GGX(NdotH, r2);
    float3 F    = F_SchlickFresnel(f0, LdotH);
    float  G    = G_SmithGGXCorrelated(NdotL, NdotV, r2);
    
    specularLight   = D * F * G * NdotL * 0.25;
    diffuseLight    = DisneyDiffuseModified(NdotV, NdotL, LdotH, r2);
    
    diffuseLight *= (1.0 - specularLight); // Energy conservation - normalize diffuse with inverted specular term
}

// Unreal Engine 4 BRDF - Heavily influneced by the Disney BRDF, but optimized for real-time rendering
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
void UE4BRDF(   in float3  viewDirection,
                in float3  lightDirection,
                in float3  normal,
                in float   roughness,
                in float3  f0,
                out float3 diffuseLight,
                out float3 specularLight)
{
    float r2 = roughness * roughness; // perceptual linear roughness

    float3 H = normalize(viewDirection + lightDirection);
    float  NdotL = saturate(dot(normal, lightDirection));
	float  NdotV = abs(dot(normal, viewDirection)) + _EPSILON; // avoid artifacts
    float  NdotH = saturate(dot(normal, H));
	float  VdotH = saturate(dot(viewDirection, H));

    float D = D_GGX(NdotH, r2);
    float F = F_SchlickFresnel(f0, VdotH);
    float G = G_SchlickGGX(NdotL, NdotV, r2);

    specularLight   = D * F * G * NdotL * 0.25;
    diffuseLight    = LambertDiffuse(NdotL);

    diffuseLight *= (1.0 - specularLight); // Energy conservation - normalize diffuse with inverted specular term
}

#endif // #if !defined(_BRDF_SHADER_)
