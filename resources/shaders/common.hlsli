// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

struct FrameData {
    uint frameNumber;
    float lastFrameTimeDelta;
    double timeStamp;
};
ConstantBuffer<FrameData> frameData : register(b0);

struct CameraData {
    float4x4 worldViewProjection;
    float4x4 worldView;
    float4x4 viewProjection;
    float3 cameraPosition;
    float nearZ;
    float farZ;
};
ConstantBuffer<CameraData> cameraData : register(b1);

struct ModelData {
    float4x4 modelWorld;
    float4x4 worldModel;
};
ConstantBuffer<ModelData> modelData : register(b2);

static const float PI = 3.14159265f;
