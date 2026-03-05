#include "SpritePass.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Math/Types/Calculations/Matrix3x3Calculations.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {

using namespace Component;
using namespace Math;
namespace {
Matrix4x4 sOrthographicMatrix;
}

NoEngine::Render::SpritePass::SpritePass() {

	auto size = GraphicsCore::gWindowManager.GetMainWindow()->GetWindowSize();
	sOrthographicMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(size.clientWidth), static_cast<float>(size.clientHeight), 0.1f, 100.f);

}

SpritePass::~SpritePass() {}

void SpritePass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	Collect(registry);
	Sort();
	GenerateVertices();
	Render(gfx);
}

void SpritePass::Collect(ECS::Registry& registry) {
	auto view = registry.View<
		Transform2DComponent,
		SpriteComponent
	>();

	items_.clear();

	for (auto entity : view) {
		auto* sprite = registry.GetComponent<SpriteComponent>(entity);
		if (!sprite->isVisible) continue;
		auto* transform = registry.GetComponent<Transform2DComponent>(entity);

		items_.push_back({ transform, sprite });
	}
}

void SpritePass::Sort() {
	std::sort(items_.begin(), items_.end(), [](const DrawItem& a, const DrawItem& b) {
		if (a.sprite->layer != b.sprite->layer) return a.sprite->layer < b.sprite->layer;
		if (a.sprite->orderInLayer != b.sprite->orderInLayer) return a.sprite->orderInLayer < b.sprite->orderInLayer;
		if (a.sprite->textureHandle != b.sprite->textureHandle) return a.sprite->textureHandle < b.sprite->textureHandle;
		return a.sprite->color < b.sprite->color;
		});
}

void SpritePass::MakeLocalQuad(const DrawItem& item, Vector2 out[4]) {
	Vector2 size = { 1.f,1.f };

	// pivot を中心にローカル座標を作る
	Vector2 origin{ size.x * item.sprite->pivot.x, size.y * item.sprite->pivot.y };

	float left = -origin.x;
	float right = size.x - origin.x;
	float top = -origin.y;
	float bottom = size.y - origin.y;

	if (item.sprite->flipX) std::swap(left, right);
	if (item.sprite->flipY) std::swap(top, bottom);

	out[0] = { left,  top };
	out[1] = { right, top };
	out[2] = { left,  bottom };
	out[3] = { right, bottom };
}

void SpritePass::GenerateVertices() {
	vertices_.clear();
	indices_.clear();
	uint16_t indexOffset = 0;
	for (auto& item : items_) {
		auto* t = item.transform;

		Vector2 local[4];
		MakeLocalQuad(item, local);
		Quaternion rotate;
		rotate.FromAxisAngle(Vector3(0.f, 0.f, 1.f), t->rotation);
		Matrix4x4 mat;
		mat.MakeAffine({ t->scale.x,t->scale.y,0.f }, rotate, { t->translate.x,t->translate.y,0.f });

		Vector3 world[4];
		for (int i = 0; i < 4; i++) {
			world[i] = MathCalculations::Transform(Vector3(local[i].x, local[i].y, 0.f), mat);
		}

		Rect uv = item.sprite->uv;
		Vector2 uvs[4] = {
			{ uv.x,			    uv.y			 },
			{ uv.x + uv.width,  uv.y			 },
			{ uv.x,			    uv.y + uv.height },
			{ uv.x + uv.width,  uv.y + uv.height },
		};

		// 頂点追加
		for (int i = 0; i < 4; i++) {
			Vector4 worldVec4 = { world[i].x, world[i].y,0.f,1.f };
			vertices_.push_back({ worldVec4, uvs[i] });
		}

		// インデックス追加
		indices_.push_back(indexOffset + 0);
		indices_.push_back(indexOffset + 1);
		indices_.push_back(indexOffset + 2);
		indices_.push_back(indexOffset + 1);
		indices_.push_back(indexOffset + 3);
		indices_.push_back(indexOffset + 2);

		indexOffset += 4;
	}

}

void SpritePass::Render(GraphicsContext& gfx) {
	if (vertices_.empty()) return;

	std::unordered_map<std::string, uint32_t>& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Default Sprite PSO");
	gfx.SetPipelineState(GetPSO(Render::GetPSOID(L"Renderer : Default Sprite PSO")));
	gfx.SetRootSignature(GetRootSignature(Render::GetRootSignatureID(L"Renderer : Default Sprite PSO")));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &sOrthographicMatrix);
	gfx.SetDynamicVB(0, vertices_.size(), sizeof(SpriteVertex), vertices_.data());
	gfx.SetDynamicIB(indices_.size(), indices_.data());
	size_t start = 0;
	
	
	while (start < items_.size())
	{
		_declspec(align(16)) struct
		{
			float fill;
			int useMask;
			float pad[2];
		} MaskConstants;
		MaskConstants.fill = items_[start].sprite->fill;
		MaskConstants.useMask = (items_[start].sprite->useMask != 0 && items_[start].sprite->maskTextureHandle.IsValid()) ? 1 : 0;

		gfx.SetDynamicConstantBufferView(rootIndex["gMaskParams"], sizeof(MaskConstants), &MaskConstants);
		
		_declspec(align(16)) struct
		{
			Color color;
		} MaterialConstants;
		MaterialConstants.color = items_[start].sprite->color;
		gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(MaterialConstants), &MaterialConstants);

		TextureRef tex = items_[start].sprite->textureHandle;
		if (items_[start].sprite->maskTextureHandle.IsValid())
		{
			TextureRef maskTex = items_[start].sprite->maskTextureHandle;
			gfx.SetDynamicDescriptor(rootIndex["gMask"], 0, maskTex.GetSRV());
		}
		size_t end = start + 1;
		while (end < items_.size() &&
			items_[end].sprite->textureHandle == tex &&
			items_[end].sprite->color.ToRGBA8() == items_[start].sprite->color.ToRGBA8())
		{
			end++;
		}

		// この範囲でDrawCall
		uint32_t indexStart = static_cast<uint32_t>(start) * 6;
		uint32_t indexCount = static_cast<uint32_t>(end - start) * 6;


		gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, tex.GetSRV());

		gfx.DrawIndexedInstanced(indexCount, 1, indexStart, 0, 0);

		start = end;
	}

}

}
}
