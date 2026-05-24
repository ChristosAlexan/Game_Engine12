#include "LightsData.hlsli"
#include "Math_Helpers.hlsli"

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct [raypayload] RayPayload
{
    float4 color : read(caller, closesthit, miss) : write(caller, closesthit, miss);
};

cbuffer CB_Shader_Camera : register(b0)
{
    float3 cameraPos;
    float padding1;
};

cbuffer CB_RT_MeshData : register(b5, space0)
{
    uint totalVertices;
    uint totalIndices;
    uint totalEntities;
    float padding;
};


struct RTVertexData
{
    float3 position;
    float padding1;
    float2 uv;
    float2 padding2;
};

struct RTIndexData
{
    uint indices;
    float3 padding;
};


struct RTMeshDataOffsets
{
    uint vertexOffset;
    uint indexOffset;
    float2 padding;
};

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D roughMetalMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);
RWTexture2D<float4> gReflectionOutput : register(u0, space5);
Texture2D albedoTextures[] : register(t0, space10);
StructuredBuffer<RTVertexData> g_vertexData : register(t1, space11);
StructuredBuffer<RTIndexData> g_indexData : register(t2, space11);
StructuredBuffer<RTMeshDataOffsets> g_dataOffsets : register(t3, space11);
SamplerState gSampler : register(s0);
RaytracingAccelerationStructure SceneBVH : register(t0, space6);

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 halfPixel = DispatchRaysIndex().xy;
    uint2 fullPixel = halfPixel * 2;
    
    float roughness = roughMetalMaskTexture.Load(int3(fullPixel, 0)).r;
    if(roughness > 0.6f)
    {
        gReflectionOutput[launchIndex] = float4(0.0f, 0.0f, 0.0f, 1.0f);
        return;
    }
   
    float3 diffuseColor = albedoTexture.Load(int3(fullPixel, 0)).xyz;
    float3 worldPos = worldPosDepthTexture.Load(int3(fullPixel, 0)).xyz;
    float3 normal = normalTexture.Load(int3(fullPixel, 0)).xyz;
    
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
    
    gReflectionOutput[launchIndex] = float4(rayPayload.color.rgb, 1.0f);
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint entityID = InstanceID();
    uint primitiveIndex = PrimitiveIndex();
    
    RTMeshDataOffsets instance = g_dataOffsets[entityID];
    uint baseVertexOffset = instance.vertexOffset;
    uint baseIndexOffset = instance.indexOffset;
    
    uint indexAddress = (primitiveIndex * 3) + baseIndexOffset;
    
    uint i0 = g_indexData[indexAddress + 0].indices;
    uint i1 = g_indexData[indexAddress + 1].indices;
    uint i2 = g_indexData[indexAddress + 2].indices;
    
    RTVertexData v0 = g_vertexData[i0 + baseVertexOffset];
    RTVertexData v1 = g_vertexData[i1 + baseVertexOffset];
    RTVertexData v2 = g_vertexData[i2 + baseVertexOffset];
    
    float u = attr.barycentrics.x;
    float v = attr.barycentrics.y;
    float w = 1.0f - u - v;
    
    float2 interpolatedUV = (v0.uv * w) + (v1.uv * u) + (v2.uv * v);
   
    float3 color = albedoTextures[entityID].SampleLevel(gSampler, interpolatedUV, 0).rgb;
    
    payload.color = float4(color, 1.0);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(0.0, 0.0, 0.0, 1.0);
}