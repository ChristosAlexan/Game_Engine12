#include "MeshGenerators.h"
#include "AssetManager.h"

ECS::MeshData GenerateCubeMesh(ECS::EntityDesc& entityDesc)
{
	ECS::MeshData mesh;


	// +Y (Top face)
	mesh.vertices.push_back(Vertex(-1, 1, -1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, -1));
	mesh.vertices.push_back(Vertex(1, 1, -1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, -1));
	mesh.vertices.push_back(Vertex(1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, -1));
	mesh.vertices.push_back(Vertex(-1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, -1));

	// -Y (Bottom face)
	mesh.vertices.push_back(Vertex(-1, -1, 1, 0, 0, 0, -1, 0, 1, 0, 0, 0, 0, 0, 1));
	mesh.vertices.push_back(Vertex(1, -1, 1, 1, 0, 0, -1, 0, 1, 0, 0, 0, 0, 0, 1));
	mesh.vertices.push_back(Vertex(1, -1, -1, 1, 1, 0, -1, 0, 1, 0, 0, 0, 0, 0, 1));
	mesh.vertices.push_back(Vertex(-1, -1, -1, 0, 1, 0, -1, 0, 1, 0, 0, 0, 0, 0, 1));

	// +X (Right face)
	mesh.vertices.push_back(Vertex(1, 1, 1, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(1, 1, -1, 1, 0, 1, 0, 0, 0, 0, -1, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(1, -1, -1, 1, 1, 1, 0, 0, 0, 0, -1, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(1, -1, 1, 0, 1, 1, 0, 0, 0, 0, -1, 0, 0, 1, 0));

	// -X (Left face)
	mesh.vertices.push_back(Vertex(-1, 1, -1, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(-1, 1, 1, 1, 0, -1, 0, 0, 0, 0, 1, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(-1, -1, 1, 1, 1, -1, 0, 0, 0, 0, 1, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(-1, -1, -1, 0, 1, -1, 0, 0, 0, 0, 1, 0, 0, 1, 0));

	// +Z (Front face)
	mesh.vertices.push_back(Vertex(-1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(1, -1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(-1, -1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0));

	// -Z (Back face)
	mesh.vertices.push_back(Vertex(1, 1, -1, 0, 0, 0, 0, -1, -1, 0, 0, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(-1, 1, -1, 1, 0, 0, 0, -1, -1, 0, 0, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(-1, -1, -1, 1, 1, 0, 0, -1, -1, 0, 0, 0, 0, 1, 0));
	mesh.vertices.push_back(Vertex(1, -1, -1, 0, 1, 0, 0, -1, -1, 0, 0, 0, 0, 1, 0));


	mesh.indices = { 
		0,2,1, 0,3,2,    // Top face
		4,6,5, 4,7,6,    // Bottom face
		8,10,9, 8,11,10, // Right face
		12,14,13, 12,15,14, // Left face
		16,18,17, 16,19,18, // Front face
		20,22,21, 20,23,22  // Back face
	};

    return mesh;
}

ECS::MeshData GenerateQuadMesh(ECS::EntityDesc& entityDesc)
{
	ECS::MeshData mesh;
	mesh.vertices.push_back(Vertex(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f));
	mesh.vertices.push_back(Vertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f));
	mesh.vertices.push_back(Vertex(1.0f, 1.0f, 0.0f, 1.0f, 0.0f));
	mesh.vertices.push_back(Vertex(1.0f, -1.0f, 0.0f, 1.0f, 1.0f));


	mesh.indices.push_back(0);
	mesh.indices.push_back(1);
	mesh.indices.push_back(2);
	mesh.indices.push_back(0);
	mesh.indices.push_back(2);
	mesh.indices.push_back(3);

    return mesh;
}

ECS::MeshData GenerateStaticMesh(Model& model, ECS::EntityDesc& entityDesc)
{
	model.name = entityDesc.name;
	model.LoadModel(entityDesc.filePath);

	return model.GetMeshData();
}

ECS::MeshData GenerateSkeletalMesh(Model& model, ECS::EntityDesc& entityDesc)
{
	model.name = entityDesc.name;
	model.SetAnimFiles(entityDesc.anim_filePaths);
	model.LoadModel(entityDesc.filePath);

	return model.GetMeshData();
}


