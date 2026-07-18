cbuffer CB_VS_VertexShader : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer CB_VS_PerObject : register(b1, space0)
{
    float4x4 worldMatrixInstanced;
    uint vertexCount;
    bool hasAnim;
    uint padding0; // Keep 16-byte alignment clean
    uint padding1;
};