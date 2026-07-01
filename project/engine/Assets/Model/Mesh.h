#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"

#include "RaytracingMesh.h"

namespace NoEngine {
struct Vertex {
	Math::Vector3 position;
	Math::Vector2 texcoord;
	Math::Vector3 normal;
};

struct Node {
	Transform transform;
	Math::Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

struct JointWeightData {
	Math::Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};


struct SubMesh {
	uint32_t vertexStart = 0;
	uint32_t vertexCount = 0;
	uint32_t indexStart = 0;
	uint32_t indexCount = 0;

	uint32_t materialIndex = 0;
};

_declspec(align(16))struct SkeletonWell {
	Math::Matrix4x4 skeletonSpaceMatrix;
	Math::Matrix4x4 skeletonSpaceInverseTransposeMatrix;
};

struct Mesh {
	std::unordered_map<std::string, JointWeightData> skinClusterData;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	StructuredBuffer baseVertexBuffer;
	StructuredBuffer useVertexBuffer;
	ByteAddressBuffer indexBuffer;
	Node rootNode;
	std::vector<SubMesh> subMeshes; // モデルに複数のメッシュが含まれている場合のaiMeshごとの範囲
	uint32_t numJoints;

	std::unique_ptr<RaytracingMesh> raytracingMesh; // レイトレーシング有効時のみ生成

	std::vector<SkeletonWell> mappedPalette;
	UploadBuffer paletteUpload;
	StructuredBuffer paletteResource;
};
}