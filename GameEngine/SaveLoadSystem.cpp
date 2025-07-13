#include "SaveLoadSystem.h"
#include "EntityFactory.h"
#include "Scene.h"
#include "EntityECS.h"

namespace ECS
{
	SaveLoadSystem::SaveLoadSystem()
	{
	}

	void SaveLoadSystem::SaveScene(Scene* scene, const std::string& filepath)
	{
		nlohmann::json sceneJson;
		sceneJson["entities"] = nlohmann::json::array();

		auto group = scene->GetRegistry().group<>(entt::get<TransformComponent, RenderComponent, EntityDesc>);

		for (auto [entity, transform, renderComponent, entityDesc] : group.each())
		{
			nlohmann::json entityJson;

			entityJson["name"] = entityDesc.name;
			entityJson["meshType"] = entityDesc.meshType;
			entityJson["materialName"] = entityDesc.materialDesc.name;
			if (entityDesc.meshType != ECS::MESH_TYPE::LIGHT)
			{
				entityJson["filePath"] = entityDesc.filePath;
				entityJson["hasAnimation"] = entityDesc.hasAnimation;
				entityJson["hasTextures"] = entityDesc.hasTextures;


				
				entityJson["albedoTextureName"] = entityDesc.materialDesc.albedoTextureName;
				entityJson["albedoTexturePath"] = entityDesc.materialDesc.albedoTexturePath;
				entityJson["normalTextureName"] = entityDesc.materialDesc.normalTextureName;
				entityJson["normalTexturePath"] = entityDesc.materialDesc.normalTexturePath;
				entityJson["metalRoughnessTextureName"] = entityDesc.materialDesc.metalRoughnessTextureName;
				entityJson["metalRoughnessTexturePath"] = entityDesc.materialDesc.metalRoughnessTexturePath;
				entityJson["useAlbedoMap"] = entityDesc.materialDesc.useAlbedoMap;
				entityJson["useNormalMap"] = entityDesc.materialDesc.useNormalMap;
				entityJson["useMetalRoughnessMap"] = entityDesc.materialDesc.useMetalRoughnessMap;
				entityJson["roughness"] = entityDesc.materialDesc.roughness;
				entityJson["metalness"] = entityDesc.materialDesc.metalness;
				entityJson["tex_format"] = entityDesc.materialDesc.tex_format;

				if (entityDesc.hasAnimation)
				{
					entityJson["AnimSize"] = entityDesc.anim_filePaths.size();
					for (int i = 0; i < entityDesc.anim_filePaths.size(); ++i)
					{
						entityJson["AnimFiles" + std::to_string(i)] = entityDesc.anim_filePaths[i];
					}
				}
			}
		
			entityJson["transform"] = {
		   {"position", {transform.position.x, transform.position.y, transform.position.z}},
		   {"rotation", {transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w}},
		   {"scale",    {transform.scale.x, transform.scale.y, transform.scale.z}}
			};


			
		
			if (entityDesc.meshType == ECS::MESH_TYPE::LIGHT)
			{
				if (scene->GetRegistry().all_of<ECS::LightComponent>(entity))
				{
					ECS::LightComponent& lightComponent = scene->GetRegistry().get<ECS::LightComponent>(entity);
					entityJson["lightType"] = lightComponent.lightType;
					entityJson["radius"] = lightComponent.radius;
					entityJson["strength"] = lightComponent.strength;
					entityJson["color"] = { lightComponent.color.x, lightComponent.color.y, lightComponent.color.z };
				}
		
			}

			sceneJson["entities"].push_back(entityJson);
		}

		std::ofstream out(filepath);
		out << sceneJson.dump(4);
	}

	void SaveLoadSystem::LoadScene(ECS::Scene* scene, const std::string& filepath)
	{
		std::ifstream in(filepath);
		nlohmann::json sceneJson;
		in >> sceneJson;

		for (const auto& entityJson : sceneJson["entities"])
		{
			TransformComponent transform;
			EntityDesc entityDesc;

			entityDesc.name = entityJson["name"];
			entityDesc.meshType = entityJson["meshType"];
			entityDesc.materialName = entityJson["materialName"];
			entityDesc.materialDesc.name = entityJson["materialName"];
	
			if(entityDesc.meshType != ECS::MESH_TYPE::LIGHT)
			{
				entityDesc.filePath = entityJson["filePath"];
				entityDesc.hasAnimation = entityJson["hasAnimation"];
				entityDesc.hasTextures = entityJson["hasTextures"];
			
				entityDesc.materialDesc.albedoTextureName = entityJson["albedoTextureName"];
				entityDesc.materialDesc.albedoTexturePath = entityJson["albedoTexturePath"];
				entityDesc.materialDesc.normalTextureName = entityJson["normalTextureName"];
				entityDesc.materialDesc.normalTexturePath = entityJson["normalTexturePath"];
				entityDesc.materialDesc.metalRoughnessTextureName = entityJson["metalRoughnessTextureName"];
				entityDesc.materialDesc.metalRoughnessTexturePath = entityJson["metalRoughnessTexturePath"];

				entityDesc.materialDesc.roughness = entityJson["roughness"];
				entityDesc.materialDesc.metalness = entityJson["metalness"];
				entityDesc.materialDesc.tex_format = entityJson["tex_format"];


				entityDesc.materialDesc.useAlbedoMap = entityJson["useAlbedoMap"];
				entityDesc.materialDesc.useNormalMap = entityJson["useNormalMap"];
				entityDesc.materialDesc.useMetalRoughnessMap = entityJson["useMetalRoughnessMap"];

				if (entityDesc.hasAnimation)
				{
					int animSize = entityJson["AnimSize"];
					entityDesc.anim_filePaths.resize(animSize);

					for (int i = 0; i < entityDesc.anim_filePaths.size(); ++i)
					{
						entityDesc.anim_filePaths[i] = entityJson["AnimFiles" + std::to_string(i)];
					}
				}
			}
			
			const auto& trans = entityJson["transform"];

			entityDesc.transform.position = { trans["position"][0], trans["position"][1], trans["position"][2] };
			entityDesc.transform.rotation = { trans["rotation"][0], trans["rotation"][1], trans["rotation"][2], trans["rotation"][3] };
			entityDesc.transform.scale = { trans["scale"][0], trans["scale"][1], trans["scale"][2] };

		
			if (entityDesc.meshType == ECS::MESH_TYPE::LIGHT)
			{
				entityDesc.lightComponent.lightType = entityJson["lightType"];
				entityDesc.lightComponent.radius = entityJson["radius"];
				entityDesc.lightComponent.strength = entityJson["strength"];
				const auto& color = entityJson["color"];
				entityDesc.lightComponent.color = { color[0], color[1], color[2] };
				entityDesc.materialDesc.baseColor = entityDesc.lightComponent.color;
			}

			scene->GetEntityFactory()->AddEntity(scene, entityDesc);
		}
	}
}
