#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"

namespace NoEngine {
struct Vertex {
	Vector4 position;
	Vector2 texcoord;
};

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	ByteAddressBuffer vertexBuffer;
	ByteAddressBuffer indexBuffer;
	Node rootNode;
};
}