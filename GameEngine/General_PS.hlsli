cbuffer CB_PS_Material : register(b1)
{
    float4 color; // 16 bytes
    float roughness; // 4 bytes
    float metalness; // 4 bytes
    bool hasTextures; // 1 byte
    bool useAlbedo; // 1 byte
    bool useNormals; // 1 byte
    bool useRoughnessMetal; // 1 byte
    float padding; // 4 bytes
};

cbuffer CB_PS_Camera : register(b2)
{
    float3 cameraPos;
    float padding1;
};

struct GPULight
{
    float3 position;
    float strength;

    float3 color;
    float radius;
    
    float4 direction;
    
    uint lighType;
    float cutoff;
    float2 padding;
};