#pragma once
#include <cstdint>
#include <vector>


struct Primitive {
	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t materialIndex;
};

// Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
struct Mesh {
	std::vector<Primitive> primitives;
};

// A node represents an object in the glTF scene graph
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