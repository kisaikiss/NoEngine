#include "SpritePass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Runtime/GraphicsCore.h"


namespace NoEngine {
namespace Render {

using namespace Component;
using namespace Math;
namespace {
Matrix4x4 sCameraMatrix;
}

NoEngine::Render::SpritePass::SpritePass() {

	auto size = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	sCameraMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(size.clientWidth), static_cast<float>(size.clientHeight), 0.1f, 100.f);

}

SpritePass::~SpritePass() {}

void SpritePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	(void)resourceRegistry;
	CameraUpdate();
	Collect(registry);
	Sort();
	GenerateVertices(registry);
	Render(gfx);
}

void SpritePass::CameraUpdate() {
	auto* camera = GetTargetCamera2D();
	if (camera) {
		sCameraMatrix = camera->viewProjection;
	} else {
		Vector2 windowSize = GraphicsCore::GetWindowSize();
		sCameraMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(windowSize.x), static_cast<float>(windowSize.y), 0.1f, 100.f);
	}
	
}

void SpritePass::Collect(ECS::Registry& registry) {
	auto view = registry.View<Transform2DComponent, SpriteComponent>();

	worldItems_.clear();
	screenItems_.clear();

	for (auto entity : view) {
		auto* sprite = registry.GetComponent<SpriteComponent>(entity);
		if (!sprite->isVisible || !sprite->textureHandle.IsValid()) continue;
		auto* transform = registry.GetComponent<Transform2DComponent>(entity);

		if (sprite->space == SpriteSpace::Screen) {
			screenItems_.push_back({ transform, sprite });
		} else {
			worldItems_.push_back({ transform, sprite });
		}
	}
}

void SpritePass::Sort() {
	auto comparator = [](const DrawItem& a, const DrawItem& b) {
		if (a.sprite->layer != b.sprite->layer) return a.sprite->layer < b.sprite->layer;
		if (a.sprite->orderInLayer != b.sprite->orderInLayer) return a.sprite->orderInLayer < b.sprite->orderInLayer;
		if (a.sprite->textureHandle != b.sprite->textureHandle) return a.sprite->textureHandle < b.sprite->textureHandle;
		return a.sprite->color < b.sprite->color;
		};
	std::sort(worldItems_.begin(), worldItems_.end(), comparator);
	std::sort(screenItems_.begin(), screenItems_.end(), comparator);
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

void SpritePass::GenerateVertices(ECS::Registry& registry) {
	vertices_.clear();
	indices_.clear();
	uint16_t indexOffset = 0;

	auto appendItems = [&](const std::vector<DrawItem>& items, bool isScreenSpace) {
		Vector2 windowSize = GraphicsCore::GetWindowSize();

		for (auto& item : items) {
			auto* t = item.transform;

			Vector2 local[4];
			MakeLocalQuad(item, local);
			Matrix4x4 mat = t->MakeAffineMatrix4x4(registry);

			Vector3 world[4];
			for (int i = 0; i < 4; i++) {
				world[i] = MathCalculations::Transform(Vector3(local[i].x, local[i].y, 0.f), mat);
				if (isScreenSpace) {
					// アンカー基準でウィンドウ座標へオフセット
					world[i].x += item.sprite->anchor.x * windowSize.x;
					world[i].y += item.sprite->anchor.y * windowSize.y;
				}
			}

			Rect uv = item.sprite->uv;
			Vector2 uvs[4] = {
				{ uv.x,             uv.y              },
				{ uv.x + uv.width,  uv.y              },
				{ uv.x,             uv.y + uv.height  },
				{ uv.x + uv.width,  uv.y + uv.height  },
			};

			for (int i = 0; i < 4; i++) {
				Vector4 worldVec4 = { world[i].x, world[i].y, 0.f, 1.f };
				vertices_.push_back({ worldVec4, uvs[i] });
			}

			indices_.push_back(indexOffset + 0);
			indices_.push_back(indexOffset + 1);
			indices_.push_back(indexOffset + 2);
			indices_.push_back(indexOffset + 1);
			indices_.push_back(indexOffset + 3);
			indices_.push_back(indexOffset + 2);

			indexOffset += 4;
		}
		};

	appendItems(worldItems_, false);
	screenIndexOffset_ = indices_.size(); // ここからScreen分
	appendItems(screenItems_, true);

}

void SpritePass::Render(GraphicsContext& gfx) {
	if (vertices_.empty()) return;
	auto* renderCtx = GetRenderContext();
	std::unordered_map<std::string, uint32_t>& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Default Sprite PSO");
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : Default Sprite PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : Default Sprite PSO"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicVB(0, vertices_.size(), sizeof(SpriteVertex), vertices_.data());
	gfx.SetDynamicIB(indices_.size(), indices_.data());

	// --- World空間: カメラ行列を使用 ---
	if (!worldItems_.empty()) {
		gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &sCameraMatrix);
		DrawBatchRange(gfx, worldItems_, rootIndex, 0);
	}

	// --- Screen空間(UI): 常に現在のウィンドウサイズ基準、カメラ無視 ---
	if (!screenItems_.empty()) {
		Vector2 windowSize = GraphicsCore::GetWindowSize();
		Matrix4x4 screenMatrix = MathCalculations::MakeOrthographicMatrix(
			0.f, 0.f, windowSize.x, windowSize.y, 0.1f, 100.f);
		gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &screenMatrix);
		DrawBatchRange(gfx, screenItems_, rootIndex, static_cast<uint32_t>(screenIndexOffset_ / 6));
	}
}

void SpritePass::DrawBatchRange(GraphicsContext& gfx, const std::vector<DrawItem>& items, std::unordered_map<std::string, uint32_t>& rootIndex, uint32_t itemBaseIndex) {
	size_t start = 0;
	while (start < items.size()) {
		_declspec(align(16)) struct {
			float fill;
			int useMask;
			float angle;
			float pad;
		} MaskConstants;
		MaskConstants.fill = items[start].sprite->fill;
		MaskConstants.useMask = (items[start].sprite->useMask != 0 && items[start].sprite->maskTextureHandle.IsValid()) ? 1 : 0;
		MaskConstants.angle = items[start].sprite->nonRenderAngle;
		gfx.SetDynamicConstantBufferView(rootIndex["gMaskParams"], sizeof(MaskConstants), &MaskConstants);

		_declspec(align(16)) struct {
			Color color;
		} MaterialConstants;
		MaterialConstants.color = items[start].sprite->color;
		gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(MaterialConstants), &MaterialConstants);

		TextureRef tex = items[start].sprite->textureHandle;
		if (items[start].sprite->maskTextureHandle.IsValid()) {
			TextureRef maskTex = items[start].sprite->maskTextureHandle;
			gfx.SetDynamicDescriptor(rootIndex["gMask"], 0, maskTex.GetSRV());
		}

		size_t end = start + 1;
		while (end < items.size() &&
			items[end].sprite->textureHandle == tex &&
			items[end].sprite->color.ToRGBA8() == items[start].sprite->color.ToRGBA8()) {
			end++;
		}

		uint32_t indexStart = (itemBaseIndex + static_cast<uint32_t>(start)) * 6;
		uint32_t indexCount = static_cast<uint32_t>(end - start) * 6;

		gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, tex.GetSRV());
		gfx.DrawIndexedInstanced(indexCount, 1, indexStart, 0, 0);

		start = end;
	}
}

}
}
