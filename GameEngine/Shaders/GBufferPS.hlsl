#include "General_PS.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 worldPos : WORLD_POSITION;
    float4 boneweights : TEXCOORD1;
};

struct GBufferOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 roughMetalMask : SV_Target2; // Mask is used to exclude pixels from lighting calculations and just output the color(useful for lights)
    float4 worldPosDepth : SV_Target3;
};

Texture2D albedoTexture : register(t0, space1);
Texture2D normalTexture : register(t1, space1);
Texture2D metalRougnessMaskTexture : register(t2, space1);
SamplerState gSampler : register(s0);

GBufferOutput Main(PSInput input)
{
    GBufferOutput output;
    float3 worldPos = input.worldPos;
    float4 albedo = float4(albedoTexture.Sample(gSampler, input.uv).rgb, 1.0f);
    float3 normal = normalTexture.Sample(gSampler, input.uv).xyz;
    normal = normalize(normal * 2.0f - 1.0f);
    // TBN matrix
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent.xyz);
    float sign = input.tangent.w;
    float3 B = sign * normalize(cross(N, T)); // reconstruct binormals

    float3x3 TBN = float3x3(T, B, N);

    float3 worldNormal = normalize(mul(normal, TBN));
    
    float metalness = metalRougnessMaskTexture.Sample(gSampler, input.uv).b;
    float roughness = metalRougnessMaskTexture.Sample(gSampler, input.uv).g;
    float depth = input.position.z / input.position.w;
    
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