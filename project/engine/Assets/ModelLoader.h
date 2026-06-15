#pragma once
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "Model/ModelAsset.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace NoEngine {
namespace Asset {

class ModelLoader {
public:
	static void LoadModel(const std::string& name, const std::string& filePath, Component::MeshComponent* model = nullptr, Component::AnimatorComponent* animator = nullptr);
	static ModelAsset Load(const std::string& filePath);
	static void GetModel(const std::string& name, Component::MeshComponent* model, Component::AnimatorComponent* animator = nullptr);
	static std::span<Material> GetMaterial(const std::string& name);
	static void DeleteAll();

private:

	static Node ReadNode(aiNode* node);
	static void ProcessAnimation(const std::string& name, const aiScene* scene);
	static std::vector<AnimationHandle> ProcessAnimation(const aiScene* scene);
	static void ProcessSkeleton(const std::string& name, const Node& rootNode);
	static SkeletonHandle ProcessSkeleton(const Node& rootNode, Mesh& mesh);
	static RaytracingMesh ProcessRaytracingMesh(const Mesh& mesh);
	static int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);

	static RaytracingGeometry MakeGeometryDesc(const Mesh& mesh, const SubMesh& sm);
};

}
}
