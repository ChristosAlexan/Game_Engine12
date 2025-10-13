cbuffer CB_VS_VertexShader : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer CB_VS_Skinning : register(b1)
{
    uint vertexCount; // 4 bytes
    float3 padding2; // 12 bytes
    bool hasAnim; // 1 byte
};