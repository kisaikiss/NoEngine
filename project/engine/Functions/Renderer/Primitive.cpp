#include"Primitive.h"
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Functions/Shader/ShaderModule.h"

using namespace NoEngine;
using namespace NoEngine::Math;
namespace {
struct PrimitiveVertex {
	Vector3 pos;
	Color color;
};

RootSignature sRootSig;
GraphicsPSO* pPSO = nullptr;
RootSignature* pRootSig = nullptr;
bool sIsInitialized = false;

std::vector<PrimitiveVertex> sVertices;
std::vector<PrimitiveVertex> sVertices2D;
}

void DebugPrimitive::Initialize(RenderContext& ctx) {
	if (sIsInitialized) return;
	pPSO = &ctx.GetGraphicsPSO("Renderer : Primitive PSO");
	pRootSig = &ctx.GetRootSignature("Renderer : Primitive PSO");
	sIsInitialized = true;
}

void DebugPrimitive::FrameEnd() {
	sVertices.clear();
	sVertices2D.clear();
}

void DebugPrimitive::DrawLine(const Vector3& a, const Vector3& b, const Color& color) {
	AddLineInternal(a, b, color);
}

void DebugPrimitive::DrawLine2D(const Math::Vector2& a, const Math::Vector2& b, const Math::Color& color) {
	Add2DLineInternal(a, b, color);
}

void DebugPrimitive::DrawCube(const Vector3& center, const Vector3& size,
	const Color& color) {
	Vector3 h = size * 0.5f;

	Vector3 p[8] =
	{
		{ center.x - h.x,  center.y - h.y,  center.z - h.z }, { center.x + h.x, center.y - h.y, center.z - h.z },
		{ center.x + h.x,  center.y + h.y,  center.z - h.z }, { center.x - h.x, center.y + h.y, center.z - h.z },
		{ center.x - h.x,  center.y - h.y,  center.z + h.z }, { center.x + h.x, center.y - h.y, center.z + h.z },
		{ center.x + h.x,  center.y + h.y,  center.z + h.z }, { center.x - h.x, center.y + h.y, center.z + h.z },
	};

	constexpr uint16_t edges[][2] =
	{
		{0,1},{1,2},{2,3},{3,0},
		{4,5},{5,6},{6,7},{7,4},
		{0,4},{1,5},{2,6},{3,7},
	};

	for (auto& e : edges)
		AddLineInternal(p[e[0]], p[e[1]], color);
}

void DebugPrimitive::DrawCube(const Vector3& center, const Vector3& size,
	const Quaternion& rotation, const Color& color) {
	Vector3 h = size * 0.5f;

	// ローカル空間でのオフセット（回転前）
	Vector3 localOffsets[8] =
	{
		{ -h.x, -h.y, -h.z }, { h.x, -h.y, -h.z },
		{  h.x,  h.y, -h.z }, { -h.x,  h.y, -h.z },
		{ -h.x, -h.y,  h.z }, { h.x, -h.y,  h.z },
		{  h.x,  h.y,  h.z }, { -h.x,  h.y,  h.z },
	};

	Vector3 p[8];
	for (int i = 0; i < 8; i++) {
		p[i] = center + (rotation.RotateVector(localOffsets[i]));
	}

	constexpr uint16_t edges[][2] =
	{
		{0,1},{1,2},{2,3},{3,0},
		{4,5},{5,6},{6,7},{7,4},
		{0,4},{1,5},{2,6},{3,7},
	};

	for (auto& e : edges)
		AddLineInternal(p[e[0]], p[e[1]], color);
}

void DebugPrimitive::DrawCube2D(const Vector2& center, const Vector2& max, const Vector2& min, const Color& color) {
	Vector2 p[4] = {
		{center.x + min.x, center.y + min.y}, {center.x + max.x, center.y + min.y},
		{center.x + max.x, center.y + max.y}, {center.x + min.x, center.y + max.y},
	};

	Add2DLineInternal(p[0], p[1], color);
	Add2DLineInternal(p[1], p[2], color);
	Add2DLineInternal(p[2], p[3], color);
	Add2DLineInternal(p[3], p[0], color);
}

void DebugPrimitive::DrawCube2D(const Math::Vector2& center, const Math::Vector2& max, const Math::Vector2& min, float angle, const Math::Color& color) {
	Vector2 localCorners[4] = {
	{ min.x, min.y }, { max.x, min.y },
	{ max.x, max.y }, { min.x, max.y },
	};

	float c = std::cos(angle);
	float s = std::sin(angle);

	Vector2 p[4];
	for (int i = 0; i < 4; i++) {
		float rx = localCorners[i].x * c - localCorners[i].y * s;
		float ry = localCorners[i].x * s + localCorners[i].y * c;
		p[i] = { center.x + rx, center.y + ry };
	}

	Add2DLineInternal(p[0], p[1], color);
	Add2DLineInternal(p[1], p[2], color);
	Add2DLineInternal(p[2], p[3], color);
	Add2DLineInternal(p[3], p[0], color);
}

void DebugPrimitive::DrawSphere(const Vector3& center, float radius, const Color& color, uint32_t slices, uint32_t stacks) {
	if (slices < 3) slices = 3;
	if (stacks < 2) stacks = 2;

	const float dPhi = PI / stacks;
	const float dTheta = PI * 2.0f / slices;

	for (uint32_t stack = 1; stack < stacks; stack++) {
		float phi = stack * dPhi;

		for (uint32_t slice = 0; slice < slices; slice++) {
			float theta0 = dTheta * slice;
			float theta1 = dTheta * (slice + 1);

			Vector3 p0{
				center.x + radius * std::sin(phi) * std::cos(theta0),
				center.y + radius * std::cos(phi),
				center.z + radius * std::sin(phi) * std::sin(theta0)
			};

			Vector3 p1{
				center.x + radius * std::sin(phi) * std::cos(theta1),
				center.y + radius * std::cos(phi),
				center.z + radius * std::sin(phi) * std::sin(theta1)
			};

			AddLineInternal(p0, p1, color);
		}
	}

	for (uint32_t slice = 0; slice < slices; slice++) {
		float theta = dTheta * slice;

		for (uint32_t stack = 0; stack < stacks; stack++) {
			float phi0 = dPhi * stack;
			float phi1 = dPhi * (stack + 1);

			Vector3 p0{
				center.x + radius * std::sin(phi0) * std::cos(theta),
				center.y + radius * std::cos(phi0),
				center.z + radius * std::sin(phi0) * std::sin(theta)
			};

			Vector3 p1{
				center.x + radius * std::sin(phi1) * std::cos(theta),
				center.y + radius * std::cos(phi1),
				center.z + radius * std::sin(phi1) * std::sin(theta)
			};

			AddLineInternal(p0, p1, color);
		}
	}
}

void DebugPrimitive::DrawTriangle(
	const Vector3& a,
	const Vector3& b,
	const Vector3& c,
	const Color& color) {
	AddLineInternal(a, b, color);
	AddLineInternal(b, c, color);
	AddLineInternal(c, a, color);
}

void DebugPrimitive::Render(GraphicsContext& ctx, const Matrix4x4& ViewProj) {
	if (sVertices.empty() || !pPSO) return;

	ctx.SetRootSignature(*pRootSig);
	ctx.SetPipelineState(*pPSO);
	ctx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	ctx.SetDynamicVB(0, sVertices.size(), sizeof(PrimitiveVertex), sVertices.data());

	ctx.SetDynamicConstantBufferView(0, sizeof(ViewProj), &ViewProj);

	ctx.DrawInstanced((uint32_t)sVertices.size(), 1, 0, 0);
}

void DebugPrimitive::Render2D(GraphicsContext& ctx, const Math::Matrix4x4& ViewProj) {
	if (sVertices2D.empty() || !pPSO) return;

	ctx.SetRootSignature(*pRootSig);
	ctx.SetPipelineState(*pPSO);
	ctx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	ctx.SetDynamicVB(0, sVertices2D.size(), sizeof(PrimitiveVertex), sVertices2D.data());

	ctx.SetDynamicConstantBufferView(0, sizeof(ViewProj), &ViewProj);

	ctx.DrawInstanced((uint32_t)sVertices2D.size(), 1, 0, 0);
}

void DebugPrimitive::AddLineInternal(const Vector3& a, const Vector3& b, const Color& color) {
	sVertices.push_back({ a, color });
	sVertices.push_back({ b, color });
}

void DebugPrimitive::Add2DLineInternal(const Math::Vector2& a, const Math::Vector2& b, const Math::Color& color) {
	sVertices2D.push_back({ Math::Vector3(a.x,a.y,0.f),color });
	sVertices2D.push_back({ Math::Vector3(b.x,b.y,0.f),color });
}
