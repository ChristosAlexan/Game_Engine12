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

// Hardcoded cube vertex positions (36 vertices = 12 triangles)
//static const float3 cubeVertices[36] =
//{
//    float3(+1, -1, -1), float3(+1, -1, +1), float3(+1, +1, +1),
//    float3(+1, -1, -1), float3(+1, +1, +1), float3(+1, +1, -1),
//    // -X face
//    float3(-1, -1, +1), float3(-1, -1, -1), float3(-1, +1, -1),
//    float3(-1, -1, +1), float3(-1, +1, -1), float3(-1, +1, +1),
//    // +Y face
//    float3(-1, +1, -1), float3(+1, +1, -1), float3(+1, +1, +1),
//    float3(-1, +1, -1), float3(+1, +1, +1), float3(-1, +1, +1),
//    // -Y face
//    float3(-1, -1, +1), float3(+1, -1, +1), float3(+1, -1, -1),
//    float3(-1, -1, +1), float3(+1, -1, -1), float3(-1, -1, -1),
//    // +Z face
//    float3(-1, -1, +1), float3(-1, +1, +1), float3(+1, +1, +1),
//    float3(-1, -1, +1), float3(+1, +1, +1), float3(+1, -1, +1),
//    // -Z face
//    float3(+1, -1, -1), float3(+1, +1, -1), float3(-1, +1, -1),
//    float3(+1, -1, -1), float3(-1, +1, -1), float3(-1, -1, -1),
//};

PSInput Main(VSInput input)
{
    PSInput output;
    
    output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, float4(input.position.xyz, 1.0f))));
    //output.position = float4(input.position, 1.0f);
    output.worldDir = normalize(mul(projectionMatrix, float4(input.position.xyz, 0.0f)).xyz);
    return output;
}
