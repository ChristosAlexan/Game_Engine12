#pragma once
#include <DirectXMath.h>

//VERTEX SHADERS
struct CB_VS_SimpleShader
{
	DirectX::XMMATRIX worldMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;
	//DirectX::XMMATRIX skinningMatrix[100];
};
struct CB_VS_AnimationShader
{
	DirectX::XMMATRIX skinningMatrix[100];
	bool HasAnim;
};


// PIXEL SHADERS
struct CB_PS_SimpleShader
{
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 lightPos;
};