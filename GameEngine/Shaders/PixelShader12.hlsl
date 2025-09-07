cbuffer CB_PS_PixelShader : register(b0)
{
    float4 color;
    float4 lightPos;
};

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


Texture2D albedoTexture : register(t0, space1);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float3 albedo = albedoTexture.Sample(gSampler, input.uv).rgb;
 
    return float4(albedo, 1.0f);
}