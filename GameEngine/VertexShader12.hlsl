// BasicShader.hlsl

#include "General.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 boneWeights : BONEWEIGHTS;
    uint4 boneIndices : BONEINDICES;
};
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 worldPos : WORLD_POSITION;
    float4 boneweights : TEXCOORD1;
};

PSInput Main(VSInput input)
{
    PSInput output;
    
    if(hasAnim)
    {
        float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        Weights[0] = input.boneWeights.x;
        Weights[1] = input.boneWeights.y;
        Weights[2] = input.boneWeights.z;
        Weights[3] = input.boneWeights.w;
    
        float3 skinnedPos = float3(0.0f, 0.0f, 0.0f);
        float3 n = float3(0.0f, 0.0f, 0.0f);
        float3 t = float3(0.0f, 0.0f, 0.0f);
        float3 b = float3(0.0f, 0.0f, 0.0f);
    
        for (int i = 0; i < 4; i++)
        {
            skinnedPos += Weights[i] * mul(skinningMatrices[input.boneIndices[i]], float4(input.position, 1.0f)).xyz;
            n += Weights[i] * mul(skinningMatrices[input.boneIndices[i]], float4(input.normal, 0.0f)).xyz;
            t += Weights[i] * mul(skinningMatrices[input.boneIndices[i]], float4(input.tangent, 0.0f)).xyz;
            b += Weights[i] * mul(skinningMatrices[input.boneIndices[i]], float4(input.binormal, 0.0f)).xyz;
    
        }
    
        output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, float4(skinnedPos, 1.0f))));
        output.normal = normalize(mul(worldMatrix, float4(n, 0.0f))); //set normalMatrix's w to 0 to omit translation with worldMatrix
        output.tangent = normalize(mul(worldMatrix, float4(t, 0.0f)));
        output.binormal = normalize(mul(worldMatrix, float4(b, 0.0f)));
   
        output.worldPos = mul(worldMatrix, float4(skinnedPos, 1.0f));
    
        float4 debugColor = float4(skinningMatrices[input.boneIndices[0]][0][0], 0.0f, 0.0f, 1.0f);
        output.boneweights = float4(Weights[0], Weights[1], Weights[2], Weights[3]);
    }
    else
    {
        output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, float4(input.position, 1.0f))));
        output.normal = normalize(mul(worldMatrix, float4(input.normal, 0.0f))); //set normalMatrix's w to 0 to omit translation with worldMatrix
        output.tangent = normalize(mul(worldMatrix, float4(input.tangent, 0.0f)));
        output.binormal = normalize(mul(worldMatrix, float4(input.binormal, 0.0f)));
   
        output.worldPos = mul(worldMatrix, float4(input.position, 1.0f));
    }
  
    output.uv = input.uv;
 
    return output;
}
