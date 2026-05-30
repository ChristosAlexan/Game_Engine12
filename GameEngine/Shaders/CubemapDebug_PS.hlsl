#include "General_PS.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 worldDir : TEXCOORD1;
};

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D roughMetalMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);
TextureCube cubeMapTexture : register(t4, space0);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float2 screenUV = input.position.xy / screenSize.xy;
    float gDepth = worldPosDepthTexture.SampleLevel(gSampler, screenUV, 0).a;
    
    if (gDepth < 0.999f)
    {
        discard;
    }
    
    float3 dir = normalize(input.worldDir);
    return float4(cubeMapTexture.SampleLevel(gSampler, dir, 0).rgb, 1.0f);
}