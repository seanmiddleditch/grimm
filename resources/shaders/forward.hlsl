//--------------------------------------------------------------------------------------
// The base forward lighting shader. All forward lighting is calculated in world space.
//--------------------------------------------------------------------------------------

#if !defined(_FORWARD_SHADER_)
#define _FORWARD_SHADER_

#include "core/common.hlsli"
#include "core/lighting.hlsli"
#include "posteffects/tonemapping.hlsli"

//#define DEBUG_WORLD_NORMAL
//#define DEBUG_WORLD_TANGENT
//#define DEBUG_BASECOLOR
//#define DEBUG_GLOSS
//#define DEBUG_SPECULAR
//#define DEBUG_AO
//#define DEBUG_VERT_COLOR

//#define DEBUG_DIFFUSE_LIGHT
//#define DEBUG_SPECULAR_LIGHT
//#define DEBUG_AMBIENT_LIGHT
//#define DEBUG_EMISSIVE_LIGHT

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

cbuffer cbCamera : register(b0)
{
	matrix g_mCamView;
	matrix g_mCamProj;
	float3 g_vCamPos;
	float2 g_vCamClipPlane;
};

cbuffer cbObject : register(b1)
{
	matrix mViewProj;
	matrix mWorld;
};

// Ambient light
static const float	k_ambientIntensity = 1.0;

// Directional light
static const float3 k_lightPos 			= float3(-100.0, 100.0, -200.0);
static const float3 k_lightColor 		= float3(1.0, 1.0, 1.0);
static const float  k_lightIntensity 	= 1.0;

static const float horizonOcclusion	= 1.3;	

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------

TextureCube g_txSpecularCube 	: register(t0);
TextureCube g_txIrradianceCube 	: register(t1);
Texture2D 	g_txSmithDFGLUT 	: register(t2);

//--------------------------------------------------------------------------------------
// Standard Input/Output Structures
//--------------------------------------------------------------------------------------

struct VS_INPUT
{
	float4 vPosition		: POSITION;
	float3 vNormal			: NORMAL;
	float3 vTangent			: TANGENT;
	float2 vTexcoord0		: TEXCOORD0;
	float4 vVertexColor		: COLOR;
};

struct VS_OUTPUT
{
	float4 vPosition    	: SV_POSITION;
	float3 vWorldPosition	: POSITION1;
	float3 vWorldNormal		: NORMAL;
	float3 vWorldTangent	: TANGENT;
	float2 vTexcoord0    	: TEXCOORD0;
	float4 vVertexColor		: COLOR;
	float3 vViewDirection	: POSITION2;
};

//--------------------------------------------------------------------------------------
// Custom Surface Shading Input/Output Stuctures
//--------------------------------------------------------------------------------------

struct VS_INPUT_OUTPUT
{
	float4 vWorldPosition;
	float3 vWorldNormal;
	float3 vWorldTangent;
	float2 vTexcoord0;
	float4 vVertexColor;
};

struct PS_INPUT_OUTPUT
{
	float3 vWorldPosition;
	float3 vWorldNormal;
	float3 vWorldTangent;
	float2 vTexcoord0;
	float4 vVertexColor;

	float3 baseColor;
	float  roughness;
	float  metalness;
	float  ao;
	float  specular;
	float  opacity;

	float3 emissive;
};

// Forward declare custom surface shading functions
void CustomVertexShaderMain	(inout VS_INPUT_OUTPUT vs_in_out);
void CustomPixelShaderMain	(inout PS_INPUT_OUTPUT ps_in_out);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

VS_OUTPUT vs_main(VS_INPUT Input)
{
	// Object/Model space -> World space
	float4 vWorldPosition	= mul(Input.vPosition, mWorld);
	float3 vWorldNormal		= mul(Input.vNormal, (float3x3)mWorld);
	float3 vWorldTangent	= mul(Input.vTangent, (float3x3)mWorld);

	// Initialize the vertex Input/Output structure and fill out the values
	VS_INPUT_OUTPUT vs_in_out;

	vs_in_out.vWorldPosition	= vWorldPosition;
	vs_in_out.vWorldNormal		= vWorldNormal;
	vs_in_out.vWorldTangent		= vWorldTangent;
	vs_in_out.vTexcoord0		= Input.vTexcoord0;
	vs_in_out.vVertexColor		= Input.vVertexColor;

//--------------------------------------------------------------------------------------
#if defined(CUSTOM_VERTEX_SHADER_MAIN)
	CustomVertexShaderMain(vs_in_out); // Evaluate custom vertex surface shading code
#endif
//--------------------------------------------------------------------------------------

	// Initialize the VS Output structure and fill out the values
	VS_OUTPUT Output;

	// vPosition world -> view -> proj
	Output.vPosition = mul(vs_in_out.vWorldPosition, mul(g_mCamProj, g_mCamView));
	
	// The rest of the vertex data stays in world space
	Output.vWorldPosition 	= vs_in_out.vWorldPosition.xyz;
	Output.vWorldNormal		= vs_in_out.vWorldNormal;
	Output.vWorldTangent	= vs_in_out.vWorldTangent;
	Output.vTexcoord0		= vs_in_out.vTexcoord0;
	Output.vVertexColor		= vs_in_out.vVertexColor;

	// Lastly, calculate the view direction in world space
	Output.vViewDirection	= g_vCamPos - vs_in_out.vWorldPosition.xyz;

	// Return final output
	return Output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 ps_main(VS_OUTPUT Input) : SV_TARGET
{
	// Initialize the pixel Input/Output structure and fill out initial values
	PS_INPUT_OUTPUT ps_in_out;

	ps_in_out.vWorldPosition	= Input.vWorldPosition;
	ps_in_out.vWorldNormal 		= normalize(Input.vWorldNormal);
	ps_in_out.vWorldTangent 	= normalize(Input.vWorldTangent);
	ps_in_out.vTexcoord0 		= Input.vTexcoord0;
	ps_in_out.vVertexColor 		= Input.vVertexColor;

	ps_in_out.baseColor		= 0.5;
	ps_in_out.roughness		= 0.3;
	ps_in_out.metalness		= 0.0;
	ps_in_out.ao			= 1.0;
	ps_in_out.specular		= 0.5; // Maps to 0.04 specular reflectance at normal incidence (f0)
	ps_in_out.opacity		= 1.0;

	ps_in_out.emissive		= 0.0;

//--------------------------------------------------------------------------------------
#if defined(CUSTOM_PIXEL_SHADER_MAIN)
	CustomPixelShaderMain(ps_in_out); // Evaluate custom pixel surface shading code
#endif
//--------------------------------------------------------------------------------------

	// Ensure Analytical specular highlights are still visible at high gloss values
	ps_in_out.roughness = max(0.05, ps_in_out.roughness);

	const float F0_CAP = 0.16; // Max specular reflectance at normal incidence (f0) - 16%
	float specular = ps_in_out.specular * ps_in_out.specular * F0_CAP;

	// Metalness workflow
	float3 diffuseColor 	= lerp(ps_in_out.baseColor, 0.0, ps_in_out.metalness);
	float3 specularColor 	= lerp(specular, ps_in_out.baseColor, ps_in_out.metalness);

//--------------------------------------------------------------------------------------
// Start Direct Analytical Lighting
//--------------------------------------------------------------------------------------

	float3 f0 	= specularColor;

	// Initialize lighting values
	float3 diffuseLight		= 0.0;
	float3 specularLight	= 0.0;
	float3 ambientLight		= 0.0;
	float3 emissiveLight	= 0.0;

	const float3 lightDirection = normalize(k_lightPos); // directional light vector
	//const float3 lightDirection = normalize(k_lightPos - ps_in_out.vWorldPosition); // point light vector

	float3 viewDirection = normalize(Input.vViewDirection);

	CalculateForwardLighting(	viewDirection,
								lightDirection,
								ps_in_out.vWorldNormal,
								ps_in_out.roughness,
								f0,
								diffuseLight,
								specularLight);

	// Micro-Shadow
	float microShadow = MicroShadow(ps_in_out.ao, ps_in_out.vWorldNormal, lightDirection);
	//microShadow = 1.0;

	// Temp: Apply light intensity and color - Eventually this will move to the lighting loop
	diffuseLight 	*= k_lightIntensity * k_lightColor * microShadow;
	specularLight 	*= k_lightIntensity * k_lightColor * microShadow;

//--------------------------------------------------------------------------------------
// Start Indirect IBL Lighting
//--------------------------------------------------------------------------------------
	
	float  NdotV = abs(dot(ps_in_out.vWorldNormal, viewDirection)) + _EPSILON; 
	float3 F = F_SchlickFresnelRoughness(f0, NdotV, ps_in_out.roughness);
	
	// DFG BRDF LUT
	float2 dfg_lut = g_txSmithDFGLUT.SampleLevel(SS_BILINEAR_CLAMP, float2(NdotV, ps_in_out.roughness), 0).rg;
	float3 dfg = EnvDFGLUT(dfg_lut, F);

	// Analytical DFG approximation
	//float3 dfg = EnvDFGPolynomial(f0, ps_in_out.roughness, NdotV);

	// Indirect Specular
	float3 worldReflect	= normalize(reflect(-viewDirection, ps_in_out.vWorldNormal));
	float specularOcclusion = SpecularHorizonOcclusion(worldReflect, normalize(Input.vWorldNormal), horizonOcclusion);

	float3 indirectSpecular = g_txSpecularCube.SampleLevel(SS_TRILINEAR_CLAMP, worldReflect, ps_in_out.roughness * 6).rgb;
	indirectSpecular *= dfg; // Apply DFG
	indirectSpecular *= specularOcclusion;

	specularLight += indirectSpecular * k_ambientIntensity;
	
	// Indirect Diffuse
	float3 indirectDiffuse = g_txIrradianceCube.SampleLevel(SS_BILINEAR_CLAMP, ps_in_out.vWorldNormal, 0).rgb;
		
	float3 kD = 1.0 - F;
	kD *= 1.0 - ps_in_out.metalness;

	indirectDiffuse *= kD; // Energy conservation - normalize diffuse with inverted specular contribution
	
	// AO plus diffuse Horizon occlusion (EXPERIMENTAL!)
	float diffuseOcclusion = DiffuseHorizonOcclusion(ps_in_out.vWorldNormal, normalize(Input.vWorldNormal), horizonOcclusion);
	indirectDiffuse *= min(ps_in_out.ao, diffuseOcclusion);

	ambientLight += indirectDiffuse * k_ambientIntensity;

//--------------------------------------------------------------------------------------
// Start Debug output modes
//--------------------------------------------------------------------------------------

#if defined(DEBUG_BASECOLOR)
	return float4(ps_in_out.baseColor, 1.0);
#endif

#if defined(DEBUG_WORLD_NORMAL)
	return float4(ps_in_out.vWorldNormal * 0.5 + 0.5, 1.0);
#endif

#if defined(DEBUG_WORLD_TANGENT)
	return float4(ps_in_out.vWorldTangent * 0.5 + 0.5, 1.0);
#endif

#if defined(DEBUG_GLOSS)
	return ps_in_out.roughness;
#endif

#if defined(DEBUG_SPECULAR)
	return ps_in_out.specular;
#endif

#if defined(DEBUG_AO)
	return ps_in_out.ao;
	//return microShadow;
#endif

#if defined(DEBUG_VERT_COLOR)
	return Input.vVertexColor;
#endif 

	// Debug lighting
	
#if defined(DEBUG_DIFFUSE_LIGHT)
	return float4(diffuseLight, 1.0);
#endif

#if defined(DEBUG_SPECULAR_LIGHT)
	return float4(specularLight, 1.0);
#endif

#if defined(DEBUG_AMBIENT_LIGHT)
	return float4(ambientLight, 1.0);
#endif

#if defined(DEBUG_EMISSIVE_LIGHT)
	return float4(emissiveLight, 1.0);
#endif

//--------------------------------------------------------------------------------------
// End debug output modes
//--------------------------------------------------------------------------------------
	
	float alpha = ps_in_out.opacity;
	float specularAlpha = 1.0;

#if !defined(PBR_ALPHA)
	specularAlpha = alpha;
#endif

	// Calculate final diffuse color
	float4 color = float4(diffuseColor * (diffuseLight + ambientLight), 1.0);
	color = float4(color.rgb * alpha, alpha); // Pre-multiplied alpha
	
	// Add specular and emissive light at the end
	color.rgb += specularLight * specularAlpha;
	color.rgb += emissiveLight * specularAlpha;

//--------------------------------------------------------------------------------------
// !!Temp: Tonemapping on a per pixel basis. This needs to move to a screen based post effect!!
	
	//color.rgb = Reinhard(color.rgb);
	//color.rgb = Uncharted2(color.rgb);
	color.rgb = ACESFitted(color.rgb);
//--------------------------------------------------------------------------------------
	
	return color;
}
#endif // !define(_FORWARD_SHADER_)
