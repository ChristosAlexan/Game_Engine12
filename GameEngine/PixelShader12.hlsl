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
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 worldPos : WORLD_POSITION;
    float4 boneweights : TEXCOORD1;
};


Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metalRougnessTexture : register(t2);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float3 worldPos = input.worldPos;
    float3 albedo = albedoTexture.Sample(gSampler, input.uv).rgb;
    float3 normal = normalize(normalTexture.Sample(gSampler, input.uv).xyz * input.normal);
    float metalness = metalRougnessTexture.Sample(gSampler, input.uv).r;
    float rougness = metalRougnessTexture.Sample(gSampler, input.uv).g;
  
    
    float3 lightDir = normalize(float3(lightPos.x, lightPos.y, lightPos.z));
    float diffuse = max(dot(normal, lightDir), 0.0);
    return float4(albedo, 1.0f);
}