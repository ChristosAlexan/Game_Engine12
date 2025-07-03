#pragma once

#include "MeshData.h"
#include "MaterialManager.h"
#include "ModelData.h"
#include <tiny_gltf.h>

class Model
{
public:
	Model();
	bool LoadModel(const std::string& filepath);
	void LoadSkeleton(tinygltf::Model& input);
	void LoadAnimations(tinygltf::Model& input);
	Node* FindNode(Node* parent, uint32_t index);
	Node* NodeFromIndex(uint32_t index);
	DirectX::XMMATRIX GetNodeMatrix(Node* node);
	void UpdateAnimation(float deltaTime, std::vector<DirectX::XMMATRIX>& finalTransform);
	void UpdateJoints(Node* node, std::vector<DirectX::XMMATRIX>& finalTransform);
	void CalculateFinalTransform(float dt, std::vector<DirectX::XMMATRIX>& finalTransform);
	ECS::MeshData& GetMeshData();

	void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, uint32_t nodeIndex);

private:


public:
	std::string name;

private:
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;


	ECS::MeshData m_cpuMesh;
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	std::vector<DirectX::XMMATRIX> inverseBindMatrices;
	std::unordered_map<int, int> nodeToJointMap;

	std::vector<Node*> nodes;
	std::vector<Skin> skins;
	std::vector<Animation> animations;

	uint32_t activeAnimation = 0;

	bool bLoaded = false;
};

