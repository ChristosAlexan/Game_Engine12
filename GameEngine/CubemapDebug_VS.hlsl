#include "General_VS.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 worldDir : TEXCOORD1;
};

PSInput Main(VSInput input)
{
    PSInput output;
    float4 worldPos = float4(input.position.xyz, 1.0f);
    
    output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, float4(worldPos.xyz, 1.0f))));
    output.worldDir = input.position.xyz;
    output.uv = input.uv;
    return output;
}
