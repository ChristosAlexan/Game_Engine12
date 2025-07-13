#pragma once
#include "DX12Includes.h"
#include <cstdint>
#include "Texture12.h"

namespace ECS
{
    enum TEXTURE_TYPE
    {
        DIFFUSE = 0,
        NORMAL = 1,
        METAL_ROUGHNESS = 2
    };

    struct MaterialDesc
    {
        std::string name = "defaultMaterial";

        std::string albedoTextureName = "defaultAlbedo";    // albedo (diffuse) texture
        std::string normalTextureName = "defaultNormal";    // normal map
        std::string metalRoughnessTextureName = ""; // roughness map


        std::string albedoTexturePath = "";    // albedo (diffuse) texture
        std::string normalTexturePath = "";    // normal map
        std::string metalRoughnessTexturePath = ""; // roughness map
        Texture12::TEXTURE_FORMAT tex_format;

        DirectX::XMFLOAT3 baseColor = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f); // Used if no albedo texture
        float roughness = 0.5f;
        float metalness = 0.0f;

        bool useAlbedoMap = false;
        bool useNormalMap = false;
        bool useMetalRoughnessMap = false;

        static MaterialDesc Default()
        {
            return MaterialDesc{};
        }
    };

    struct Material {
        std::string name;
        std::shared_ptr<Texture12> albedoTexture;    // albedo (diffuse) texture
        std::shared_ptr<Texture12> normalTexture;    // normal map
        std::shared_ptr<Texture12> metalRoughnessTexture; // roughness map

        std::vector<std::shared_ptr<Texture12>> textures;

        Texture12::TEXTURE_FORMAT tex_format;
        DirectX::XMFLOAT4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // Used if no albedo texture
        float roughness = 0.5f;
        float metalness = 0.0f;

        bool useAlbedoMap = false;
        bool useNormalMap = false;
        bool useMetalRoughnessMap = false;
    };
}