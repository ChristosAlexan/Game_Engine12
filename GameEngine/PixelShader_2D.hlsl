
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D roughMetalTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float4 color = albedoTexture.Sample(gSampler, input.uv).rgba;
    float3 normal = normalTexture.Sample(gSampler, input.uv).xyz;
    normal = normalize(normal.xyz * 2.0 - 1.0);
    return float4(color.rgb, 1.0f);
}