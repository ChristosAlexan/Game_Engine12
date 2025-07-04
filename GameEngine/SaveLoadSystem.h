#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <vector>

namespace ECS
{
	class Scene;
	class SaveLoadSystem
	{
	public:
		SaveLoadSystem();
		void SaveScene(Scene* scene, const std::string& filepath);
		void LoadScene(Scene* scene, const std::string& filepath);
	};
}


