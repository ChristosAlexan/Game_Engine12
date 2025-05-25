#pragma once
#include "DX12Includes.h"
#include <cstdint>

namespace ECS
{
    struct Material {
        uint32_t albedoTextureID = 0;    // Index to albedo (diffuse) texture
        uint32_t normalTextureID = 0;    // Index to normal map
        uint32_t roughnessTextureID = 0; // Index to roughness map
        uint32_t metalnessTextureID = 0; // Index to metalness map

        DirectX::XMFLOAT4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // Used if no albedo texture
        float roughness = 0.5f;
        float metalness = 0.0f;

        bool useAlbedoMap = false;
        bool useNormalMap = false;
        bool useRoughnessMap = false;
        bool useMetalnessMap = false;
    };
}