#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include "TransformECS.h"

struct Primitive {
	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t materialIndex;
};

struct Mesh {
	std::vector<Primitive> primitives;
};

struct Node {
	Node* parent;
	uint32_t			index;
	std::vector<Node*> children;
	Mesh mesh;
	DirectX::XMFLOAT3 translation = DirectX::XMFLOAT3(0.0f,0.0f,0.0f);
	DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	int32_t             skin = -1;
	DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();

	DirectX::XMMATRIX getLocalMatrix();
	
	~Node() {
		for (auto& child : children) {
			delete child;
		}
	}
};

struct Skin
{
	std::string	name;
	Node* skeletonRoot = nullptr;
	std::vector<DirectX::XMMATRIX> inverseBindMatrices;
	std::vector<Node*>    joints;
	std::vector<DirectX::XMMATRIX> finalTransforms;
};


struct AnimationSampler
{
	std::string            interpolation;
	std::vector<float>     inputs;
	std::vector<DirectX::XMVECTOR> outputsVec4;
};

struct AnimationChannel
{
	std::string path;
	Node* node;
	uint32_t    samplerIndex;
};

struct Animation
{
	std::string                   name;
	std::vector<AnimationSampler> samplers;
	std::vector<AnimationChannel> channels;
	float                         start = std::numeric_limits<float>::max();
	float                         end = std::numeric_limits<float>::min();
	float                         currentTime = 0.0f;
};


struct FlatNode
{
	DirectX::XMFLOAT3 translation = { 0, 0, 0 };
	DirectX::XMFLOAT4 rotation = { 0, 0, 0, 1 };
	DirectX::XMFLOAT3 scale = { 1, 1, 1 };
	int parentIndex = -1; // -1 means root
};

struct AnimatorComponent
{
	int currentAnim = 0;
	int previousAnim = 0;
	float currentTime = 0.0f;
	std::vector<FlatNode> flatNodes;
	std::vector<FlatNode> flatNodesPrev;
	std::vector<DirectX::XMMATRIX> finalTransforms;
	bool isBlending = false;
	float blendDuration = 0.25f;
	float blendTime = 0.0f;
};
