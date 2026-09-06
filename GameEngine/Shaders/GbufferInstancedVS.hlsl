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
    uint vertexID : SV_VertexID;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 worldPos : WORLD_POSITION;
    nointerpolation uint meshDataIndex : MESHDATAINDEX;
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

StructuredBuffer<SkinningDataOut> g_skinningData : register(t1, space8);

struct InstanceData
{
    float4x4 worldMatrix;
    uint meshDataIndex;
    float3 pad;
};
StructuredBuffer<InstanceData> g_InstanceTransforms : register(t0, space9);

PSInput Main(VSInput input, uint instanceID : SV_InstanceID)
{
    PSInput output;

    InstanceData instanceData = g_InstanceTransforms[instanceID];
    float4x4 worldMat = instanceData.worldMatrix;

    output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMat, float4(input.position, 1.0f))));
    output.normal = normalize(mul(worldMat, float4(input.normal, 0.0f)));
    output.tangent = normalize(mul(worldMat, float4(input.tangent.xyz, 0.0f)));
    output.binormal = normalize(mul(worldMat, float4(input.binormal, 0.0f)));
    output.worldPos = mul(worldMat, float4(input.position, 1.0f));

    output.uv = input.uv;
    output.meshDataIndex = instanceData.meshDataIndex;

    return output;
}