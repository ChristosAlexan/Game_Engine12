#include "LightsData.hlsli"
#include "Math_Helpers.hlsli"

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct [raypayload] RayPayload
{
    float4 color : read(caller, closesthit, miss) : write(caller, closesthit, miss);
};

struct Attributes
{
    float2 barycentric;
};

cbuffer CB_Shader_Camera : register(b0)
{
    float3 cameraPos;
    float padding1;
};
Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D metalRoughnessMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);
RWTexture2D<float4> gReflectionOutput : register(u0, space5);
RaytracingAccelerationStructure SceneBVH : register(t0, space6);

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 dim = DispatchRaysDimensions().xy;
    if (any(launchIndex >= dim))
        return;
    
    float3 diffuseColor = albedoTexture.Load(int3(launchIndex, 0)).xyz;
    float3 worldPos = worldPosDepthTexture.Load(int3(launchIndex, 0)).xyz;
    float3 normal = normalTexture.Load(int3(launchIndex, 0)).xyz;
    
    float3 V = normalize(cameraPos - worldPos);
    float3 R = reflect(-V, normal);
    
    RayDesc ray;
    ray.Origin = worldPos + normal * 0.001f;
    ray.Direction = normalize(R);
    ray.TMin = 0.001f;
    ray.TMax = 1000;
    
    RayPayload rayPayload;
    rayPayload.color = float4(0, 0, 0, 1);
    
    TraceRay(SceneBVH, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, rayPayload);
    
    gReflectionOutput[launchIndex] = float4(rayPayload.color.rgb * diffuseColor.rgb, 1.0f);
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = float4(1.0, 1.0, 1.0, 1.0);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(0.0, 0.0, 0.0, 1.0);
}