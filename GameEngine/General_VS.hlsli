cbuffer CB_VS_VertexShader : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CB_VS_Skinning : register(b1)
{
    matrix skinningMatrices[100];
    bool hasAnim;
    float3 padding2; //pad to next 16-byte boundary
};