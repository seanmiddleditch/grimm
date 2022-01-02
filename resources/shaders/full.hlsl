#include "common.hlsli"

struct VS_Input {
    float3 position : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD0;
};

SamplerState bilinearSampler : register(s1);
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D texture2 : register(t2);

struct VS_Output {
    float4 position : SV_Position;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float3x3 tangentSpace : TANGENT;
    float2 uv : TEXCOORD0;
};

VS_Output vertex_main(VS_Input input) {
    VS_Output output;
    output.position = float4(input.position, 1);
    output.position = mul(output.position, modelWorld);
    output.position = mul(output.position, worldView);
    output.position = mul(output.position, viewProjection);

    output.normal = mul(mul(float4(input.normal, 0), modelWorld), worldView).xyz;
    float3 tangent = mul(mul(float4(input.tangent, 0), modelWorld), worldView).xyz;
    float3 bitangent = normalize(float4(cross(output.normal.xyz, tangent.xyz).xyz, 0)).xyz;

    output.tangentSpace = float3x3(tangent, bitangent, output.normal);

    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 pixel_main(VS_Output input) : SV_Target {
    // Normal map
    float3 mappedNormal = texture1.Sample(bilinearSampler, input.uv).xyz * 2 - 1;
    mappedNormal = mul(mappedNormal, input.tangentSpace);
    mappedNormal = normalize(mappedNormal);

    float3 normal = normalize(input.normal);
    float3 blendedNormal = normalize(float3(mappedNormal.xy + normal.xy, mappedNormal.z*normal.z));

    // AO
    float3 shadow = texture2.Sample(bilinearSampler, input.uv).rgb;

    // Diffuse
    float3 color = texture0.Sample(bilinearSampler, input.uv).rgb;

    // Calculate lighting
    float light = max(0, dot(blendedNormal, float3(0, 1, 1)));
    float3 final = color * shadow * light;

    return float4(clamp(final, 0, 1), 1);
}
