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