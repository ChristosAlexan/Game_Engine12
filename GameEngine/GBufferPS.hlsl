#include "General_PS.hlsli"

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

struct GBufferOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 roughMetalDepth : SV_Target2;
    float4 worldPos : SV_Target3;
};

Texture2D albedoTexture : register(t0, space1);
Texture2D normalTexture : register(t1, space1);
Texture2D metalRougnessTexture : register(t2, space1);
SamplerState gSampler : register(s0);

GBufferOutput Main(PSInput input)
{
    GBufferOutput output;
    float3 worldPos = input.worldPos;
    float4 albedo = float4(albedoTexture.Sample(gSampler, input.uv).rgb, 1.0f);
    float3 normal = normalize(normalTexture.Sample(gSampler, input.uv).xyz * input.normal);
    float metalness = metalRougnessTexture.Sample(gSampler, input.uv).r;
    float roughness = metalRougnessTexture.Sample(gSampler, input.uv).g;
    float depth = input.position.z / input.position.w;
    
    if(hasTextures)
    {
        output.albedo = albedo;
        output.normal = float4(normal, 1.0f);
        output.roughMetalDepth = float4(roughness, metalness, 0.0f, 0.0f);
    }
    else
    {
        output.albedo = float4(color.rgb, 1.0f);
        output.normal = float4(0.0f, 0.0f, 0.0f, 1.0f);
        output.roughMetalDepth = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    output.worldPos = float4(worldPos, depth);
    
    return output;
}