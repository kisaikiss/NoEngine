#include "stdafx.h"
#include "MeshColliderLoader.h"
#include "engine/Utilities/FileUtilities.h"


namespace NoEngine {
namespace Math {
void LoadMeshCollider(const std::string& filePath, TerrainMesh* outTerrain) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FixInfacingNormals
	);

	assert(scene->HasMeshes());
	std::string directoryPath = Utilities::GetBasePath(filePath);

    BuildTerrainMeshFromAssimpScene(scene, outTerrain->triangles);
    outTerrain->bvhRoot = BuildBVH(outTerrain->triangles);
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