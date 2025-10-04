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

PSInput Main(VSInput input)
{
    PSInput output;
    
    if(hasAnim)
    {
        if (input.vertexID < vertexCount)
        {
            SkinningDataOut skinnedData = g_skinningData[input.vertexID];
        
            output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, float4(skinnedData.position, 1.0f))));
            output.normal = normalize(mul(worldMatrix, float4(skinnedData.normal, 0.0f)));
            output.tangent = normalize(mul(worldMatrix, float4(skinnedData.tangent, 0.0f)));
            output.worldPos = mul(worldMatrix, float4(skinnedData.position, 1.0f));
        }
    }
    else
    {
        output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, float4(input.position, 1.0f))));
        output.normal = normalize(mul(worldMatrix, float4(input.normal, 0.0f)));
        output.tangent = normalize(mul(worldMatrix, float4(input.tangent.xyz, 0.0f)));
        output.worldPos = mul(worldMatrix, float4(input.position, 1.0f));
    }

    output.uv = input.uv;
 
    return output;
}
