#pragma once
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace NoEngine {
class ModelLoader {
public:
	static Mesh* LoadModel(const std::string& name, const std::string& filePath);
	static Mesh* GetModel(const std::string& name);
	static void DeleteAll();
private:
	static Node ReadNode(aiNode* node);
};

}
