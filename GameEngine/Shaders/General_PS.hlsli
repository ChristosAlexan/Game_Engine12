cbuffer CB_PS_Material : register(b1, space0)
{
    float4 color;
    float roughness;
    float metalness;
    uint hasTextures;
    uint useAlbedo;
    uint useNormals;
    uint useRoughnessMetal;
    uint meshDataIndex;
    uint3 padding;
};

cbuffer CB_PS_Camera : register(b2)
{
    float3 cameraPos;
    float padding1;
    float2 screenSize;
    float padding2;
};

cbuffer CB_PS_PBR : register(b3)
{
    float mip_roughness;
    float3 ambientColor;
    float4 exposureGamma;
};
