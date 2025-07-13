#pragma once
#include <DirectXMath.h>

/************ VERTEX SHADERS *******************/
struct CB_VS_SimpleShader
{
	DirectX::XMMATRIX worldMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;
};
struct CB_VS_AnimationShader
{
	DirectX::XMMATRIX skinningMatrix[100];
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