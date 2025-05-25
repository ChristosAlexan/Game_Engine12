// BasicShader.hlsl
cbuffer CB_VS_VertexShader : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
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

PSInput Main(VSInput input)
{
    PSInput output;
    output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, float4(input.position, 1.0f))));
    output.uv = input.uv;
    
    output.tangent = normalize(mul(float4(input.tangent, 0.0f), worldMatrix)).xyz;
    output.binormal = normalize(mul(float4(input.binormal, 0.0f), worldMatrix)).xyz;
    
    output.normal = normalize(mul(float4(input.normal, 0.0f), worldMatrix)).xyz;
    output.worldPos = mul(float4(input.position, 1.0f), worldMatrix).xyz;
    
    return output;
}
