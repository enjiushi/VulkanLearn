#pragma once
#include "../common/Singleton.h"
#include "Importer.hpp"
#include "scene.h"
#include <vector>
#include <memory>

class Mesh;

class AssimpSceneReader : public Singleton<AssimpSceneReader>
{
public:
	static std::vector<std::shared_ptr<Mesh>> Read(const std::string& path, std::vector<uint32_t> argumentedVAFList);
	static std::shared_ptr<Mesh> Read(const std::string& path, std::vector<uint32_t> argumentedVAFList, uint32_t meshIndex);
};