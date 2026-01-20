#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"

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

struct SubMesh {
	uint32_t vertexStart = 0;
	uint32_t vertexCount = 0;
	uint32_t indexStart = 0;
	uint32_t indexCount = 0;
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	ByteAddressBuffer vertexBuffer;
	ByteAddressBuffer indexBuffer;
	Node rootNode;
	std::vector<SubMesh> subMeshes; // モデルに複数のメッシュが含まれている場合のaiMeshごとの範囲
};
}