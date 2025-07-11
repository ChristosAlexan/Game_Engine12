
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
    float4 color = worldPosDepthTexture.Sample(gSampler, input.uv).rgba;
    
    return float4(color.www, 1.0f);
}