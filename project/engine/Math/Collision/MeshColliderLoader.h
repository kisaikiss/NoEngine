#pragma once
#include "ColliderComponent.h"
#include "BVH.h"
#include "../Types/Matrix4x4.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace NoEngine {
namespace Math {
struct TerrainMesh {
	// 配置変更のたびに変換元として使う、モデル座標の三角形。
	std::vector<TriangleCollider> localTriangles;
	// 衝突判定にはワールド座標の三角形と BVH を使う。
	std::vector<TriangleCollider> triangles;
    std::shared_ptr<BVHNode> bvhRoot;
	Matrix4x4 lastWorld = Matrix4x4::IDENTITY;
	bool worldDirty = true;
};

void LoadMeshCollider(const std::string& filePath, TerrainMesh* outTerrain);
void UpdateTerrainWorld(TerrainMesh& terrain, Matrix4x4 world);
}

// Assimp の aiVector3D -> Math::Vector3
inline Math::Vector3 ToVec3(const aiVector3D& v) {
    return { v.x, v.y, v.z };
}


inline Math::Vector3 TransformPoint(const aiMatrix4x4& m, const Math::Vector3& p) {
    float x = m.a1 * p.x + m.a2 * p.y + m.a3 * p.z + m.a4;
    float y = m.b1 * p.x + m.b2 * p.y + m.b3 * p.z + m.b4;
    float z = m.c1 * p.x + m.c2 * p.y + m.c3 * p.z + m.c4;
    return { x, y, z };
}

inline Math::Vector3 TransformNormal(const aiMatrix4x4& m, const Math::Vector3& n) {
    float x = m.a1 * n.x + m.a2 * n.y + m.a3 * n.z;
    float y = m.b1 * n.x + m.b2 * n.y + m.b3 * n.z;
    float z = m.c1 * n.x + m.c2 * n.y + m.c3 * n.z;
    Math::Vector3 out{ x, y, z };
    out = out.Normalize();
    return out;
}

void ProcessMeshTriangles(
    const aiMesh* mesh,
    const aiMatrix4x4& transform,
    std::vector<Math::TriangleCollider>& outTriangles);

void ProcessNodeRecursive(
    const aiNode* node,
    const aiScene* scene,
    const aiMatrix4x4& parentTransform,
    std::vector<Math::TriangleCollider>& outTriangles);

bool BuildTerrainMeshFromAssimpScene(
    const aiScene* scene,
    std::vector<Math::TriangleCollider>& outTriangles);

}
