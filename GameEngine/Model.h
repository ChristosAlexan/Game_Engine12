#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "MeshData.h"
#include "MaterialManager.h"

class Model
{
public:
	Model();
	void LoadModel(const std::string& filepath);
	ECS::MeshData& GetMeshData();
private:
	void ProcessNode(aiNode* node, const aiScene* scene);

	const aiScene* m_scene = nullptr;
	Assimp::Importer importer;
	ECS::MeshData m_cpuMesh;
	std::vector<Vertex> m_vertices;
	std::vector<DWORD> m_indices;
};

