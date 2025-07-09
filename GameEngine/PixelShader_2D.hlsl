
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D renderTexture : register(t0);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float3 color = renderTexture.Sample(gSampler, input.uv).rgb;
    
    return float4(color, 1.0f);
}