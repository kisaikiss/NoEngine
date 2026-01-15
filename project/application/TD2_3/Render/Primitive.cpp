#include"Primitive.h"
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Functions/Renderer/RenderSystem.h"

using namespace NoEngine;
namespace
{
	struct PrimitiveVertex
	{
		Vector3 pos;
		Color color;
	};

	RootSignature sRootSig;
	GraphicsPSO* pPSO = nullptr;

	Matrix4x4 sViewProj;

	std::vector<PrimitiveVertex> sVertices;
}

void Primitive::Initialize()
{
	pPSO = &Render::GetPSO(L"Renderer : Primitive PSO");
}

void Primitive::Shutdown()
{
	sVertices.clear();
}
void Primitive::DrawLine(const Vector3& a, const Vector3& b, const Color& color)
{
	AddLineInternal(a, b, color);
}
void Primitive::DrawCube(const Vector3& center, const Vector3& size,
	const Color& color)
{
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

void Primitive::DrawSphere(const Vector3& center, float radius, const Color& color, uint32_t slices, uint32_t stacks)
{
	if (slices < 3) slices = 3;
	if (stacks < 2) stacks = 2;

	const float dPhi = PI / stacks;
	const float dTheta = PI * 2.0f / slices;

	for (uint32_t stack = 1; stack < stacks; stack++)
	{
		float phi = stack * dPhi;

		for (uint32_t slice = 0; slice < slices; slice++)
		{
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

	for (uint32_t slice = 0; slice < slices; slice++)
	{
		float theta = dTheta * slice;

		for (uint32_t stack = 0; stack < stacks; stack++)
		{
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

void Primitive::DrawTriangle(
	const Vector3& a,
	const Vector3& b,
	const Vector3& c,
	const Color& color)
{
	AddLineInternal(a, b, color);
	AddLineInternal(b, c, color);
	AddLineInternal(c, a, color);
}

void Primitive::Render(GraphicsContext& ctx)
{
	if (sVertices.empty() || !pPSO) return;

	ctx.SetRootSignature(sRootSig);
	ctx.SetPipelineState(*pPSO);
	ctx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	ctx.SetDynamicVB(0, sVertices.size(), sizeof(PrimitiveVertex), sVertices.data());

	ctx.SetDynamicConstantBufferView(0, sizeof(sViewProj), &sViewProj);

	ctx.DrawInstanced((uint32_t)sVertices.size(), 1, 0, 0);

	sVertices.clear();
}

void Primitive::SetViewProj(const NoEngine::Matrix4x4& viewProj)
{
	sViewProj = viewProj;
}

void Primitive::AddLineInternal(const NoEngine::Vector3& a, const NoEngine::Vector3& b, const NoEngine::Color& color)
{
	sVertices.push_back({ a, color });
	sVertices.push_back({ b, color });
}
