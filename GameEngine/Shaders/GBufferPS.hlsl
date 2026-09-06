#include "General_PS.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 worldPos : WORLD_POSITION;
};

struct GBufferOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 roughMetalMask : SV_Target2; // Mask is used to exclude pixels from lighting calculations and just output the color(useful for lights)
    float4 worldPosDepth : SV_Target3;
};

struct MeshDataOffsets
{
    uint vertexOffset;
    uint indexOffset;
    uint albedoIndex;
    uint normalIndex;
    uint metalRoughnessIndex;
    uint padding;
};

Texture2D albedoTexture : register(t0, space1);
Texture2D normalTexture : register(t1, space1);
Texture2D metalRougnessMaskTexture : register(t2, space1);
Texture2D bindlessTextures[] : register(t0, space10);
StructuredBuffer<MeshDataOffsets> g_dataOffsets : register(t3, space11);
SamplerState gSampler : register(s0);

GBufferOutput Main(PSInput input)
{
    MeshDataOffsets instance = g_dataOffsets[meshDataIndex];
    
    GBufferOutput output;
    float3 worldPos = input.worldPos;
    //float4 albedo = float4(albedoTexture.Sample(gSampler, input.uv).rgb, 1.0f);
    float4 albedo = float4(bindlessTextures[NonUniformResourceIndex(instance.albedoIndex)].Sample(gSampler, input.uv).rgb, 1.0f);
    
    float3 normal = float4(bindlessTextures[NonUniformResourceIndex(instance.normalIndex)].Sample(gSampler, input.uv).rgb, 1.0f).rgb;
    normal = normalize(normal * 2.0f - 1.0f);
    
    // TBN matrix
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent.xyz);
    float sign = input.tangent.w;
    float3 B = sign * normalize(cross(N, T)); // reconstruct binormals

    float3x3 TBN = float3x3(T, B, N);

    float3 worldNormal = normalize(mul(normal, TBN));
    
    float metalness = float4(bindlessTextures[NonUniformResourceIndex(instance.metalRoughnessIndex)].Sample(gSampler, input.uv).rgb, 1.0f).b;
    float roughness = float4(bindlessTextures[NonUniformResourceIndex(instance.metalRoughnessIndex)].Sample(gSampler, input.uv).rgb, 1.0f).g;
    float depth = input.position.z;
    
    if(hasTextures)
    {
        output.albedo = albedo;
        output.normal = float4(worldNormal, 1.0f);
        output.roughMetalMask = float4(roughness, metalness, 1.0f, 0.0f);
    }
    else
    {
        output.albedo = float4(color.rgb, 1.0f);
        output.normal = float4(0.0f, 0.0f, 0.0f, 1.0f);
        output.roughMetalMask = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    output.worldPosDepth = float4(worldPos, depth);

    return output;
}