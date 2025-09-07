#include "General_VS.hlsli"

struct VSInput
{
    float3 position : POSITION;
};
struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldDir : TEXCOORD0;
};

PSInput Main(VSInput input)
{
    PSInput output;
    
    output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, float4(input.position.xyz, 1.0f))));
    output.worldDir = normalize(mul(projectionMatrix, float4(input.position.xyz, 0.0f)).xyz);
    return output;
}
