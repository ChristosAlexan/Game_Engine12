#include "General_PS.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D roughMetalTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);

// space2: Lights
StructuredBuffer<GPULight> g_Lights : register(t0, space2);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float4 color = albedoTexture.Sample(gSampler, input.uv).rgba;
    float3 normal = normalTexture.Sample(gSampler, input.uv).xyz;
    float3 worldPos = worldPosDepthTexture.Sample(gSampler, input.uv).xyz;
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    normal = normalize(normal.xyz * 2.0 - 1.0);
    
    float3 result = ambient;
    for (uint i = 0; i < 2; ++i)
    {
        GPULight light;
        float3 toLight = worldPos.xyz - g_Lights[i].positionViewSpace;
        float dist = length(toLight);
        float atten = saturate(1.0 - dist / g_Lights[i].radius);
        result += g_Lights[i].color * atten * g_Lights[i].strength;
    }
    
    return float4(result * color.rgb, 1.0f);
}