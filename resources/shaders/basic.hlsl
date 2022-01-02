#include "common.hlsli"

struct VS_Input {
    float3 position : POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD0;
};

sampler linearSampler : register(s0);
Texture2D texture0 : register(t0);

struct VS_Output {
    float4 position : SV_Position;
    float3 color : COLOR;
    float2 uv : TEXCOORD0;
};

VS_Output vertex_main(VS_Input input) {
    VS_Output output;
    output.position = float4(input.position, 1);
    output.position = mul(output.position, modelWorld);
    output.position = mul(output.position, worldView);
    output.position = mul(output.position, viewProjection);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 pixel_main(VS_Output input) : SV_Target {
    return float4(texture0.Sample(sampler0, input.uv).rgb, 1);
}
