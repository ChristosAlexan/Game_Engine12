#pragma once

#include "MeshData.h"
#include "MaterialManager.h"
#include "ModelData.h"
#include "TransformECS.h"
#include <tiny_gltf.h>

class Model
{
public:
	Model();

	//NOTE: Use blender 2.83 when exporting .gltf or .glb models for correct skinning, later versions are not supported for now
	bool LoadModel(const std::string& filepath);
	void SetAnimFiles(const std::vector<std::string>& animFiles);
	void LoadSkeleton(tinygltf::Model& input);
	void LoadAnimations(tinygltf::Model& input);
	Node* FindNode(Node* parent, uint32_t index);
	Node* NodeFromIndex(uint32_t index);
	DirectX::XMMATRIX GetNodeMatrix(Node* node);
	//void SetCurrentAnim(uint32_t curAnim);
	void UpdateAnimation(float deltaTime, AnimatorComponent& animData);
	void UpdateJoints(Node* node, std::vector<DirectX::XMMATRIX>& finalTransform);
	void CalculateFinalTransform(float dt, AnimatorComponent& animData);
	ECS::MeshData& GetMeshData();

	void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, uint32_t nodeIndex);

private:


public:
	std::string name;
	std::vector<Node*> nodes;
private:
	//tinygltf::Model model;
	std::vector<tinygltf::Model> models;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;


	ECS::MeshData m_cpuMesh;
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	std::vector<DirectX::XMMATRIX> inverseBindMatrices;
	std::unordered_map<int, int> nodeToJointMap;

	std::vector<Skin> skins;
	std::vector<Animation> animations;

	std::vector<std::string> m_animFiles;

};

