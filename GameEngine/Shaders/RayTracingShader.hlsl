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

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D metalRoughnessMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);
RWTexture2DArray<float> gShadowOutput : register(u0, space5);
RaytracingAccelerationStructure SceneBVH : register(t0, space6);
StructuredBuffer<GPULight> g_Lights : register(t0, space2);
RWStructuredBuffer<GPUShadows> g_Shadows : register(u1, space2);

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    float3 worldPos = worldPosDepthTexture.Load(int3(launchIndex, 0)).xyz;
    float3 normal = normalTexture.Load(int3(launchIndex, 0)).xyz;
    
    for (uint i = 0; i < totalLights; ++i)
    {
        float3 lightDir;
        float lightDistance;

        if (g_Lights[i].lighType == 0) // Directional light
        {
            lightDir = normalize(-g_Lights[i].direction.xyz);
            lightDistance = 10000.0f;
        }
        else if (g_Lights[i].lighType == 1) // Spot light
        {
            float4 q = normalize(g_Lights[i].direction);
            float3 forward = RotateVectorByQuaternion(float3(0, 0, 1), q);
            float3 L = normalize(g_Lights[i].position - worldPos).xyz;
            
            float theta = dot(L, normalize(-forward));
            float outerCutOff = g_Lights[i].cutoff / 3.0f;
   
            
            if (theta < outerCutOff)
            {
                gShadowOutput[uint3(launchIndex, i)] = 1.0f;
                continue;
            }
            
            lightDistance = length(g_Lights[i].position - worldPos);
            lightDir = L;
        }
        else if (g_Lights[i].lighType == 2) // point light
        {
            float3 toLight = g_Lights[i].position.xyz - worldPos.xyz;
            lightDistance = length(toLight);
            lightDir = toLight / lightDistance;
        }
        
        RayDesc ray;
        ray.Origin = worldPos + normal * 0.001f;
        ray.Direction = lightDir;
        ray.TMin = 0.001f;
        ray.TMax = lightDistance - 0.001f;
        
        RayPayload rayPayload;
        rayPayload.color = float4(1, 1, 1, 1);
    
        TraceRay(SceneBVH, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, rayPayload);
        
        float shadowFactor = rayPayload.color.r;
        gShadowOutput[uint3(launchIndex, i)] = shadowFactor;
    }
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = float4(0.0,0.0,0.0, 1);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(1, 1, 1, 1);
}