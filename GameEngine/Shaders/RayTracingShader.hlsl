#include "LightsData.hlsli"

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct [raypayload] RayPayload
{
    float4 color : read(caller, closesthit, miss) : write(caller, closesthit, miss);
};

struct Attributes
{
    float2 barycentric;
};

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D metalRoughnessMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);
RWTexture2D<float4> gOutput : register(u0, space5);
RaytracingAccelerationStructure SceneBVH : register(t0, space6);
StructuredBuffer<GPULight> g_Lights : register(t0, space2);

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    float3 worldPos = worldPosDepthTexture.Load(int3(launchIndex, 0)).xyz;
    float3 normal = normalTexture.Load(int3(launchIndex, 0)).xyz;
    
    float3 lightDir = normalize(-g_Lights[0].direction.xyz);
    RayDesc ray;
    ray.Origin = worldPos;
    ray.Direction = lightDir;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0f;
    RayPayload rayPayload;
    rayPayload.color = float4(0,0,0, 1.0f);
    
    TraceRay(SceneBVH, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, rayPayload);
    
    gOutput[launchIndex] = rayPayload.color;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = float4(0.2,0.2,0.2, 1);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(1, 1, 1, 1);
}
