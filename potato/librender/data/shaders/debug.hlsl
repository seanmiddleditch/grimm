// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

cbuffer cameraData : register(b1) {
    float4x4 worldViewProjection;
    float4x4 worldView;
    float4x4 viewProjection;
    float3 cameraPosition;
    float nearZ;
    float farZ;
};

struct VS_Input {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_Output {
    float4 position : SV_Position;
    float3 worldPosition : TEXTURE1;
    float4 color : COLOR;
};

VS_Output vertex_main(VS_Input input) {
    VS_Output output;
    output.worldPosition = input.position;
    output.position = float4(input.position, 1);
    output.position = mul(output.position, worldView);
    output.position = mul(output.position, viewProjection);
    output.color = input.color;
    return output;
}

float4 pixel_main(VS_Output input) : SV_Target {
    float depth = clamp(distance(input.worldPosition, cameraPosition) / farZ, 0, 1);
    float opacity = 1 - depth;
    return input.color * float4(1, 1, 1, opacity);
}
