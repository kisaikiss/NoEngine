#include "stdafx.h"
#include "MeshColliderLoader.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Math::TerrainMesh, "Collision")
REFLECT_STRUCT_END(NoEngine::Math::TerrainMesh)


namespace NoEngine {
namespace Math {
void LoadMeshCollider(const std::string& filePath, TerrainMesh* outTerrain) {
	if (!outTerrain) return;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FixInfacingNormals
	);

    if (!scene || !scene->HasMeshes()) {
        LogWarning("LoadMeshCollider : Mesh not found. file path : " + filePath);
        return;
    }
    std::vector<TriangleCollider> localTriangles;
    if (!BuildTerrainMeshFromAssimpScene(scene, localTriangles)) {
        LogWarning("LoadMeshCollider : Invalid scene. file path : " + filePath);
        return;
    }

    outTerrain->localTriangles = std::move(localTriangles);
    outTerrain->worldDirty = true;
    UpdateTerrainWorld(*outTerrain, Matrix4x4::IDENTITY);
}

void UpdateTerrainWorld(TerrainMesh& terrain, Matrix4x4 world) {
    // 行列の全要素を比較し、親の移動や回転・スケールの変更も検出する。
    bool worldChanged = false;
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            if (terrain.lastWorld.m[row][column] != world.m[row][column]) {
                worldChanged = true;
            }
        }
    }
    if (!terrain.worldDirty && !worldChanged) return;

    terrain.triangles.resize(terrain.localTriangles.size());
    for (size_t i = 0; i < terrain.localTriangles.size(); ++i) {
        const auto& source = terrain.localTriangles[i];
        auto& triangle = terrain.triangles[i];
        for (size_t vertex = 0; vertex < 3; ++vertex) {
            triangle.v[vertex] = world.Transform(source.v[vertex]);
        }

        // 変換後の頂点から法線を求め、非一様スケールも反映する。
        const auto edge0 = triangle.v[1] - triangle.v[0];
        const auto edge1 = triangle.v[2] - triangle.v[0];
        const auto normal = edge0.Cross(edge1);
        triangle.normal = normal.LengthSquared() > 0.0f
            ? normal.Normalize() : Vector3::ZERO;
    }

    // 空の三角形配列は BuildBVH に渡さない。
    if (terrain.triangles.empty()) {
        terrain.bvhRoot.reset();
    } else {
        terrain.bvhRoot = BuildBVH(terrain.triangles);
    }
    terrain.lastWorld = world;
    terrain.worldDirty = false;
}
}

void ProcessMeshTriangles(const aiMesh* mesh, const aiMatrix4x4& transform, std::vector<Math::TriangleCollider>& outTriangles) {
    // 事前に reserve しておく
    outTriangles.reserve(outTriangles.size() + mesh->mNumFaces);

    for (unsigned int fi = 0; fi < mesh->mNumFaces; ++fi) {
        const aiFace& face = mesh->mFaces[fi];
        if (face.mNumIndices != 3) continue;

        aiVector3D aiV0 = mesh->mVertices[face.mIndices[0]];
        aiVector3D aiV1 = mesh->mVertices[face.mIndices[1]];
        aiVector3D aiV2 = mesh->mVertices[face.mIndices[2]];

        aiV0.x *= -1.0f;
        aiV1.x *= -1.0f;
        aiV2.x *= -1.0f;

        Math::Vector3 v0 = TransformPoint(transform, ToVec3(aiV0));
        Math::Vector3 v1 = TransformPoint(transform, ToVec3(aiV1));
        Math::Vector3 v2 = TransformPoint(transform, ToVec3(aiV2));

        // 面法線（外積）を計算
        Math::Vector3 e0 = v1 - v0;
        Math::Vector3 e1 = v2 - v0;
        Math::Vector3 normal = e0.Cross(e1);
        normal = normal.Normalize();

        Math::TriangleCollider tri{};
        tri.v[0] = v0;
        tri.v[1] = v1;
        tri.v[2] = v2;
        tri.normal = normal;

        outTriangles.push_back(tri);
    }
}

void ProcessNodeRecursive(const aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform, std::vector<Math::TriangleCollider>& outTriangles) {
    aiMatrix4x4 local = parentTransform * node->mTransformation;

    // メッシュを処理
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMeshTriangles(mesh, local, outTriangles);
    }

    // 子ノードを再帰処理
    for (uint32_t c = 0; c < node->mNumChildren; ++c) {
        ProcessNodeRecursive(node->mChildren[c], scene, local, outTriangles);
    }
}

bool BuildTerrainMeshFromAssimpScene(const aiScene* scene, std::vector<Math::TriangleCollider>& outTriangles) {
    if (!scene || !scene->mRootNode) return false;

    outTriangles.clear();
    size_t totalFaces = 0;
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) { 
        totalFaces += scene->mMeshes[i]->mNumFaces;
    }
    outTriangles.reserve(totalFaces);

    aiMatrix4x4 identity; // 単位行列
    ProcessNodeRecursive(scene->mRootNode, scene, identity, outTriangles);

    return true;
}
}
