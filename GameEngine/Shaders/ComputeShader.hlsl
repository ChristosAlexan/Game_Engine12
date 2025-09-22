struct SkinningData
{
    float4 position;
    float4 weights;
    uint4 indices;
};

struct SkinningDataOut
{
    float3 position;
    float padding;
};

cbuffer CB_CS_Skinning : register(b0, space8)
{
    matrix skinningMatrices[100];
    uint vertexCount;
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
    
    for (int i = 0; i < 4; i++)
    {
        skinnedPos += Weights[i] * mul(skinningMatrices[g_skinningData[vertexID].indices[i]], float4(g_skinningData[vertexID].position.xyz, 1.0f)).xyz;
    }
    
    g_skinningDataOut[dispatchThreadID.x].position = skinnedPos;
    g_skinningDataOut[dispatchThreadID.x].padding = 0.0f;

}