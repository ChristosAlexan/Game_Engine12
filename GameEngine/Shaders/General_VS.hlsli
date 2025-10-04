cbuffer CB_VS_VertexShader : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CB_VS_Skinning : register(b1)
{
    uint vertexCount; // 4 bytes
    float3 padding2; // 12 bytes
    bool hasAnim; // 1 byte
};