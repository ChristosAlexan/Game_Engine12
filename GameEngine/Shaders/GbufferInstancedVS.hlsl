#include "General_VS.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 boneWeights : BONEWEIGHTS;
    uint4 boneIndices : BONEINDICES;
};
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 worldPos : WORLD_POSITION;
    nointerpolation uint meshDataIndex : MESHINDEX;
};

struct SkinningDataOut
{
    float3 position;
    float padding;
    float3 normal;
    float padding1;
    float3 tangent;
    float padding2;
    float3 binormal;
    float padding3;
};

cbuffer IndirectDrawArgs : register(b6)
{
    uint g_FirstInstance;
};

struct InstanceData
{
    float4x4 worldMatrix;
    uint meshDataIndex;
    uint skinnedOffset;
    float2 pad;
};

StructuredBuffer<SkinningDataOut> g_skinningData : register(t1, space8);
StructuredBuffer<InstanceData> g_InstanceData : register(t0, space9);

PSInput Main(VSInput input, uint instanceID : SV_InstanceID, uint vertexID : SV_VertexID)
{
    PSInput output;
    InstanceData instance = g_InstanceData[g_FirstInstance + instanceID];
    
    if (instance.skinnedOffset != 0xFFFFFFFF)
    {
        SkinningDataOut skinnedData = g_skinningData[instance.skinnedOffset + vertexID];
        
        output.position = mul(projectionMatrix, mul(viewMatrix, mul(instance.worldMatrix, float4(skinnedData.position, 1.0f))));
        output.normal = normalize(mul(instance.worldMatrix, float4(skinnedData.normal, 0.0f)));
        output.tangent = normalize(mul(instance.worldMatrix, float4(skinnedData.tangent, 0.0f)));
        output.worldPos = mul(instance.worldMatrix, float4(skinnedData.position, 1.0f));
    }
    else
    {
        output.position = mul(projectionMatrix, mul(viewMatrix, mul(instance.worldMatrix, float4(input.position, 1.0f))));
        output.normal = normalize(mul(instance.worldMatrix, float4(input.normal, 0.0f)));
        output.tangent = normalize(mul(instance.worldMatrix, float4(input.tangent.xyz, 0.0f)));
        output.worldPos = mul(instance.worldMatrix, float4(input.position, 1.0f));
    }
  
    output.meshDataIndex = instance.meshDataIndex; 
    output.uv = input.uv;
 
    return output;
}