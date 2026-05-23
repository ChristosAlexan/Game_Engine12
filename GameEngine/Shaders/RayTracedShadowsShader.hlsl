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
Texture2D roughMetalMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);
RWTexture2DArray<float> gShadowOutput : register(u0, space5);
RaytracingAccelerationStructure SceneBVH : register(t0, space6);
StructuredBuffer<GPULight> g_Lights : register(t0, space2);

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

float2 RandomDisk(float2 xi)
{
    float r = sqrt(xi.x);
    float phi = 6.28318530718f * xi.y;

    return float2(r * cos(phi), r * sin(phi));
}

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    float3 worldPos = worldPosDepthTexture.Load(int3(launchIndex, 0)).xyz;
    float3 normal = normalize(normalTexture.Load(int3(launchIndex, 0)).xyz);

    // TODO: Make these configurable
    const uint ShadowSamples = 4;
    const float PointSpotLightRadius = 0.01f;
    const float DirectionalAngularRadius = 0.001f;

    for (uint i = 0; i < totalLights; ++i)
    {
        float3 lightDir = g_Lights[i].position - worldPos;
        float distSq = dot(lightDir, lightDir);

        // Cull shadows
        float rCull = g_Lights[i].radius;
        float rFade = max(0.0f, rCull - g_Lights[i].cutoff);

        if (g_Lights[i].lighType != 0)
        {
            if (g_Lights[i].strength <= 0.0f || distSq > rCull * rCull)
                continue;
        }
        else
        {
            if (g_Lights[i].strength <= 0.0f)
                continue;
        }

        float lightDistance = 0.0f;

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
        else if (g_Lights[i].lighType == 2) // Point light
        {
            float3 toLight = g_Lights[i].position.xyz - worldPos.xyz;
            lightDistance = length(toLight);
            lightDir = toLight / lightDistance;
        }

        float shadowAccum = 0.0f;

        for (uint s = 0; s < ShadowSamples; ++s)
        {
            float rnd0 = Hash12(float2(launchIndex) + float(s) * 23.17f + float(i) * 11.31f);
            float rnd1 = Hash12(float2(launchIndex.yx) + float(s) * 91.73f + float(i) * 7.77f);
            
            float2 disk = RandomDisk(float2(rnd0, rnd1));

            float3 tangent;
            float3 bitangent;
            BuildONB(lightDir, tangent, bitangent);
            
            float3 sampleDir = lightDir;
            float sampleDistance = lightDistance;

            if (g_Lights[i].lighType == 0) // Directional light
            {

                sampleDir = normalize(lightDir + tangent * disk.x * DirectionalAngularRadius + bitangent * disk.y * DirectionalAngularRadius);

                sampleDistance = lightDistance;
            }
            else // Spot or point light
            {
                float3 sampledLightPos = g_Lights[i].position.xyz + tangent * disk.x * PointSpotLightRadius +  bitangent * disk.y * PointSpotLightRadius;

                float3 toSample = sampledLightPos - worldPos;
                sampleDistance = length(toSample);
                sampleDir = toSample / sampleDistance;
            }

            RayDesc ray;
            ray.Origin = worldPos + normal * 0.001f;
            ray.Direction = sampleDir;
            ray.TMin = 0.001f;
            ray.TMax = sampleDistance - 0.001f;

            RayPayload rayPayload;
            rayPayload.color = float4(1, 1, 1, 1);

            TraceRay(
                SceneBVH,
                RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
                ~0,
                0,
                1,
                0,
                ray,
                rayPayload
            );

            shadowAccum += rayPayload.color.r;
        }

        float shadowFactor = shadowAccum / float(ShadowSamples);
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