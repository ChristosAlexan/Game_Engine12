#include "Model.h"
#include <iostream>
#include "ErrorLogger.h"
#include "Vertex.h"
#include "MeshData.h"

Model::Model()
{
}

void Model::LoadModel(const std::string& filepath)
{
    m_scene = importer.ReadFile(filepath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_OptimizeMeshes |
        aiProcess_ValidateDataStructure);

    if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
    {
        std::string errorMsg = "Assimp Error: " + std::string(importer.GetErrorString());
        ErrorLogger::Log(errorMsg);
        return;
    }

    OutputDebugStringA(("Meshes: " + std::to_string(m_scene->mNumMeshes) + " || Materials: " +std::to_string(m_scene->mNumMaterials) + "\n").c_str());
    ProcessNode(m_scene->mRootNode, m_scene);

    m_cpuMesh.vertices = m_vertices;
    m_cpuMesh.indices = m_indices;
}

ECS::MeshData& Model::GetMeshData()
{
    return m_cpuMesh;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    aiMatrix4x4 aiTransform = node->mTransformation;
    DirectX::XMMATRIX nodeTransform = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&aiTransform));

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) 
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) 
        {
            Vertex v;

            v.pos = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            };
            if (mesh->HasNormals())
            {
                v.normal = DirectX::XMFLOAT3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z);
            }
            if (mesh->HasTextureCoords(0))
            {
                v.texCoord = DirectX::XMFLOAT2(
                    mesh->mTextureCoords[0][i].x,
                    1 - mesh->mTextureCoords[0][i].y);
            }
            if (mesh->HasTangentsAndBitangents())
            {
                v.tangent = DirectX::XMFLOAT3(
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z);
            }
            m_vertices.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                m_indices.push_back(face.mIndices[j]);
            }
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) 
    {
        ProcessNode(node->mChildren[i], scene);
    }
}
