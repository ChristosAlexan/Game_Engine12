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

cbuffer AOConstants : register(b5, space1)
{
    float AORadius;
    float NormalBias;
    uint OutputSlice;
    int SampleCount;
};

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D roughMetalMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);
RWTexture2DArray<float> gAOOutput : register(u0, space5);
RaytracingAccelerationStructure SceneBVH : register(t0, space6);

float Hash12(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * 0.1031f);
    p3 += dot(p3, p3.yzx + 33.33f);
    return frac((p3.x + p3.y) * p3.z);
}

void BuildONB(float3 n, out float3 t, out float3 b)
{
    float3 up = abs(n.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    t = normalize(cross(up, n));
    b = cross(n, t);
}

float3 CosineSampleHemisphere(float2 xi)
{
    float r = sqrt(xi.x);
    float phi = 6.28318530718f * xi.y;

    float x = r * cos(phi);
    float y = r * sin(phi);
    float z = sqrt(max(0.0f, 1.0f - xi.x));

    return float3(x, y, z);
}

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 halfPixel = DispatchRaysIndex().xy;
    uint2 fullPixel = halfPixel * 2;
    
    float3 worldPos = worldPosDepthTexture.Load(int3(fullPixel, 0)).xyz;
    float3 normal = normalize(normalTexture.Load(int3(fullPixel, 0)).xyz);

    float visibility = 0.0f;

    for (uint i = 0; i < SampleCount; ++i)
    {
        float rnd0 = Hash12(float2(launchIndex) + i * 23.17f);
        float rnd1 = Hash12(float2(launchIndex.yx) + i * 91.73f);

        float3 localDir = CosineSampleHemisphere(float2(rnd0, rnd1));

        float3 tangent;
        float3 bitangent;
        BuildONB(normal, tangent, bitangent);

        float3 rayDir = normalize(
        localDir.x * tangent +
        localDir.y * bitangent +
        localDir.z * normal
    );

        RayDesc ray;
        ray.Origin = worldPos + normal * NormalBias;
        ray.Direction = rayDir;
        ray.TMin = 0.001f;
        ray.TMax = AORadius;

        RayPayload payload;
        payload.color = float4(1.0, 1.0, 1.0, 1.0);

        TraceRay(
        SceneBVH,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        0xFF,
        0,
        1,
        0,
        ray,
        payload
    );

        visibility += payload.color.r;
    }

    float ao = visibility / max(1.0f, (float) SampleCount);
    gAOOutput[uint3(launchIndex, OutputSlice)] = ao;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = float4(0.0, 0.0, 0.0, 1);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(1, 1, 1, 1);
}