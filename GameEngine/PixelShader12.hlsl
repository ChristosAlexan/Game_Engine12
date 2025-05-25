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

float4 Main(PSInput input) : SV_TARGET
{
    float3 lightDir = normalize(float3(lightPos.x, lightPos.y, lightPos.z)); // for example
    float diffuse = max(dot(input.normal, lightDir), 0.0);
    return float4(diffuse.xxx * color.rgb, 1.0f);
}