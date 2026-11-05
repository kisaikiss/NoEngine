#pragma once
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "Mesh.h"
#include "ModelAsset.h"
#include "Skeleton.h"
#include "Animation.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace NoEngine {

class ModelLoader {
public:
	static ModelAsset Load(const std::string& filePath);
	// 指定した個体の変形済み頂点からBLASを生成する。省略時は共有の初期頂点を使う。
	static RaytracingMesh ProcessRaytracingMesh(const Mesh& mesh, const StructuredBuffer* vertexBuffer = nullptr);

private:

	static Node ReadNode(aiNode* node);
	static std::vector<Asset::AnimationHandle> ProcessAnimation(const aiScene* scene);
	static Asset::SkeletonHandle ProcessSkeleton(const Node& rootNode, Mesh& mesh);
	static int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);

	static RaytracingGeometry MakeGeometryDesc(const Mesh& mesh, const SubMesh& sm, const StructuredBuffer* vertexBuffer);
};

}
