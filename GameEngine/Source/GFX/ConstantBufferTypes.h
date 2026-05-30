#pragma once
#include <DirectXMath.h>

/************ VERTEX SHADERS *******************/
struct CB_VS_SimpleShader
{
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
};
struct CB_VS_AnimationShader
{
	uint32_t vertexCount;
	DirectX::XMFLOAT3 padding;
	bool HasAnim;
};


/************ PIXEL SHADERS *******************/
struct CB_PS_SimpleShader
{
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 lightPos;
};

struct CB_PS_Material
{
	DirectX::XMFLOAT4 color; // 16 bytes
	float roughness; // 4 bytes
	float metalness; // 4 bytes
	bool hasTextures; // 1 byte
	bool useAlbedo; // 1 byte
	bool useNormals; // 1 byte
	bool useRoughnessMetal; // 1 byte
	float padding; // 4 bytes
};

struct CB_Shader_Camera
{
	DirectX::XMFLOAT3 cameraPos;
	float padding1;
	DirectX::XMFLOAT2 screenSize;
	float padding2;
};

struct CB_PS_PBR
{
	float mip_roughness;
	DirectX::XMFLOAT3 ambientColor;
	DirectX::XMFLOAT4 exposureGamma;
};

struct CB_SHADER_LIGHTS
{
	uint32_t totalLights;
	DirectX::XMFLOAT3 padding3;
};

struct CB_SHADER_FXAA
{
	DirectX::XMFLOAT2 invScreenSize;
	float contrastThreshold = 0.04f;
	float blendStrength = 0.5f;
};

/************ COMPUTE SHADERS *******************/
struct CB_CS_AnimationShader
{
	DirectX::XMFLOAT4X4 skinningMatrix[100];
	uint32_t vertexCount;
	uint32_t padding[3] = { 0,0,0 };
};

/************ RAYTRACING SHADERS *******************/
struct CB_RT_MeshData
{
	uint32_t totalVertices;
	uint32_t totalIndices;
	uint32_t totalEntities;
	float padding = 0.0f;
};

struct CB_AO_Data
{
	float AORadius = 0.5f;
	float NormalBias = 0.0f;
	uint32_t OutputSlice = 0;
	int32_t SampleCount = 12;
};
