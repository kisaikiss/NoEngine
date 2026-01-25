#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"

namespace NoEngine {
struct Vertex {
	Vector4 position;
	Vector2 texcoord;
};

struct Node {
	Transform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
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
	Matrix4x4 skeletonSpaceMatrix;
	Matrix4x4 skeletonSpaceInverseTransposeMatrix;
};

struct Mesh {
	std::unordered_map<std::string, JointWeightData> skinClusterData;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	ByteAddressBuffer vertexBuffer;
	ByteAddressBuffer indexBuffer;
	Node rootNode;
	std::vector<SubMesh> subMeshes; // モデルに複数のメッシュが含まれている場合のaiMeshごとの範囲
	uint32_t numJoints;

	// ToDo : スキニングの情報はメッシュが持つものではないので、別の構造体へ移動します。
	std::vector<SkeletonWell> mappedPalette;
	UploadBuffer paletteUpload;
	StructuredBuffer paletteResource;
};
}