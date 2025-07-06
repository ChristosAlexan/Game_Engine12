#include "SaveLoadSystem.h"
#include "Scene.h"

namespace ECS
{
	SaveLoadSystem::SaveLoadSystem()
	{
	}

	void SaveLoadSystem::SaveScene(Scene* scene, const std::string& filepath)
	{
		nlohmann::json sceneJson;
		sceneJson["entities"] = nlohmann::json::array();

		auto renderGroup = scene->GetRegistry().group<TransformComponent, RenderComponent, EntityDesc>();

		for (auto [entity, transform, renderComponent, entityDesc] : renderGroup.each())
		{
			nlohmann::json entityJson;

			entityJson["name"] = entityDesc.name;
			entityJson["filePath"] = entityDesc.filePath;
			entityJson["hasAnimation"] = entityDesc.hasAnimation;
			entityJson["hasMaterial"] = entityDesc.hasMaterial;
			entityJson["meshType"] = entityDesc.meshType;
		

			entityJson["transform"] = {
		   {"position", {transform.position.x, transform.position.y, transform.position.z}},
		   {"rotation", {transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w}},
		   {"scale",    {transform.scale.x, transform.scale.y, transform.scale.z}}
			};

			entityJson["materialName"] = entityDesc.materialDesc.name;
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
			entityDesc.filePath = entityJson["filePath"];
			entityDesc.hasAnimation = entityJson["hasAnimation"];
			entityDesc.hasMaterial = entityJson["hasAnimation"];
			entityDesc.hasMaterial = entityJson["hasMaterial"];
			entityDesc.meshType = entityJson["meshType"];
			entityDesc.materialName = entityJson["materialName"];

			const auto& t = entityJson["transform"];

			entityDesc.transform.position = { t["position"][0], t["position"][1], t["position"][2] };
			entityDesc.transform.rotation = { t["rotation"][0], t["rotation"][1], t["rotation"][2], t["rotation"][3] };
			entityDesc.transform.scale = { t["scale"][0], t["scale"][1], t["scale"][2] };


			entityDesc.materialDesc.name = entityJson["materialName"];
			entityDesc.materialDesc.albedoTextureName = entityJson["albedoTextureName"];
			entityDesc.materialDesc.albedoTexturePath = entityJson["albedoTexturePath"];
			entityDesc.materialDesc.normalTextureName = entityJson["normalTextureName"];
			entityDesc.materialDesc.normalTexturePath = entityJson["normalTexturePath"];
			entityDesc.materialDesc.metalRoughnessTextureName = entityJson["metalRoughnessTextureName"];
			entityDesc.materialDesc.metalRoughnessTexturePath = entityJson["metalRoughnessTexturePath"];

			entityDesc.materialDesc.useAlbedoMap = entityJson["useAlbedoMap"];
			entityDesc.materialDesc.useNormalMap = entityJson["useNormalMap"];
			entityDesc.materialDesc.useMetalRoughnessMap = entityJson["useMetalRoughnessMap"];
			entityDesc.materialDesc.roughness = entityJson["roughness"];
			entityDesc.materialDesc.metalness = entityJson["metalness"];
			entityDesc.materialDesc.tex_format = entityJson["tex_format"];

			if (entityDesc.hasAnimation)
			{
				int animSize = entityJson["AnimSize"];
				entityDesc.anim_filePaths.resize(animSize);

				for (int i = 0; i < entityDesc.anim_filePaths.size(); ++i)
				{
					entityDesc.anim_filePaths[i] = entityJson["AnimFiles" + std::to_string(i)];
				}
			}
			

			scene->GetEntityFactory()->AddEntity(scene, entityDesc);
		}
	}
}
