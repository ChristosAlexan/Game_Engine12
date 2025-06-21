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
};


Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D rougnessTexture : register(t2);
Texture2D metalnessTexture : register(t3);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float3 worldPos = input.worldPos;
    float3 albedo = albedoTexture.Sample(gSampler, input.uv).rgb;
    float3 normal = normalize(normalTexture.Sample(gSampler, input.uv).xyz * input.normal);
    float3 rougness = rougnessTexture.Sample(gSampler, input.uv).rgb;
    float3 metalness = metalnessTexture.Sample(gSampler, input.uv).rgb;
    
    float3 lightDir = normalize(float3(lightPos.x, lightPos.y, lightPos.z));
    float diffuse = max(dot(normal, lightDir), 0.0);
    return float4(albedo, 1.0f);
}