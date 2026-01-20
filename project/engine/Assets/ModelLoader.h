#pragma once
#include "Mesh.h"
#include "Skeleton.h"
#include "Animation.h"

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
	static Animation LoadAnimation(const std::string& filePath);
	static Skeleton CreateSkeleton(const Node& rootNode);
	static int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);
};

}
