struct SkinningData
{
    float4 position;
    float4 weights;
    uint4 indices;
    float4 normal;
    float4 tangent;
    float4 binormal;
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

cbuffer CB_CS_Skinning : register(b0, space8)
{
    float4x4 skinningMatrices[100];
    uint vertexCount;
    uint3 padding;
};

RWTexture2D<float4> gOutput : register(u0, space8);
StructuredBuffer<SkinningData> g_skinningData : register(t0, space8);
RWStructuredBuffer<SkinningDataOut> g_skinningDataOut : register(u1, space8);

[numthreads(256, 1, 1)]
void Main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint vertexID = dispatchThreadID.x;
    
    if (vertexID >= vertexCount)
        return;
    
    float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Weights[0] = g_skinningData[vertexID].weights.x;
    Weights[1] = g_skinningData[vertexID].weights.y;
    Weights[2] = g_skinningData[vertexID].weights.z;
    Weights[3] = g_skinningData[vertexID].weights.w;
    
    float3 skinnedPos = float3(0.0f, 0.0f, 0.0f);
    float3 skinnedNormal = float3(0.0f, 0.0f, 0.0f);
    float3 skinnedTangent = float3(0.0f, 0.0f, 0.0f);
    float3 skinnedBinormal = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < 4; i++)
    {
        skinnedPos += Weights[i] * mul(skinningMatrices[g_skinningData[vertexID].indices[i]], float4(g_skinningData[vertexID].position.xyz, 1.0f)).xyz;
        skinnedNormal += Weights[i] * mul(skinningMatrices[g_skinningData[vertexID].indices[i]], float4(g_skinningData[vertexID].normal.xyz, 0.0f)).xyz;
        skinnedTangent += Weights[i] * mul(skinningMatrices[g_skinningData[vertexID].indices[i]], float4(g_skinningData[vertexID].tangent.xyz, 0.0f)).xyz;
        skinnedBinormal += Weights[i] * mul(skinningMatrices[g_skinningData[vertexID].indices[i]], float4(g_skinningData[vertexID].binormal.xyz, 0.0f)).xyz;
    }
    
    g_skinningDataOut[dispatchThreadID.x].position = skinnedPos;
    g_skinningDataOut[dispatchThreadID.x].padding = 0.0f;
    g_skinningDataOut[dispatchThreadID.x].normal = skinnedNormal;
    g_skinningDataOut[dispatchThreadID.x].padding1 = 0.0f;
    g_skinningDataOut[dispatchThreadID.x].tangent = skinnedTangent;
    g_skinningDataOut[dispatchThreadID.x].binormal = skinnedBinormal;
    g_skinningDataOut[dispatchThreadID.x].padding2 = 0.0f;
}