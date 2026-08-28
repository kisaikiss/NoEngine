#include "SpriteIDPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {
using namespace Component;
using namespace Math;

void SpriteIDPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	(void)resourceRegistry;

	Collect(registry);
	if (items_.empty()) return;
	Sort();

	auto* renderCtx = GetRenderContext();
	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Sprite ObjectID PSO");
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : Sprite ObjectID PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : Sprite ObjectID PSO"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto* camera = GetTargetCamera2D();
	Vector2 windowSize = GraphicsCore::GetWindowSize();

	Matrix4x4 worldCameraMatrix;
	if (camera) {
		worldCameraMatrix = camera->viewProjection;
	} else {
		worldCameraMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, windowSize.x, windowSize.y, 0.1f, 100.f);
	}
	Matrix4x4 screenCameraMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, windowSize.x, windowSize.y, 0.1f, 100.f);

	for (auto& item : items_) {
		bool isScreenSpace = item.sprite->space == SpriteSpace::Screen;

		Vector2 local[4];
		MakeLocalQuad(item, local);
		Matrix4x4 mat = item.transform->MakeAffineMatrix4x4(registry);

		SpriteVertex vertices[4];
		Rect uv = item.sprite->uv;
		Vector2 uvs[4] = {
			{ uv.x,             uv.y              },
			{ uv.x + uv.width,  uv.y              },
			{ uv.x,             uv.y + uv.height  },
			{ uv.x + uv.width,  uv.y + uv.height  },
		};
		for (int i = 0; i < 4; ++i) {
			Vector3 world = MathCalculations::Transform(Vector3(local[i].x, local[i].y, 0.f), mat);
			if (isScreenSpace) {
				world.x += item.sprite->anchor.x * windowSize.x;
				world.y += item.sprite->anchor.y * windowSize.y;
			}
			vertices[i].position = { world.x, world.y, 0.f, 1.f };
			vertices[i].texcoord = uvs[i];
		}
		std::vector<uint16_t> indices = { 0,1,2,1,3,2 };

		gfx.SetDynamicVB(0, 4, sizeof(SpriteVertex), vertices);
		gfx.SetDynamicIB(6, indices.data());

		const Matrix4x4& cameraMatrix = isScreenSpace ? screenCameraMatrix : worldCameraMatrix;
		gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &cameraMatrix);

		_declspec(align(16)) struct {
			Color id;
		} idConstants;
		idConstants.id = Color(static_cast<uint32_t>(item.entity));
		gfx.SetDynamicConstantBufferView(rootIndex["gObjectID"], sizeof(idConstants), &idConstants);

		gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, item.sprite->textureHandle.GetSRV());

		gfx.DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
}

void SpriteIDPass::Collect(ECS::Registry& registry) {
	items_.clear();
	auto view = registry.View<Transform2DComponent, SpriteComponent>();
	for (auto entity : view) {
		auto* sprite = registry.GetComponent<SpriteComponent>(entity);
		if (!sprite->isVisible || !sprite->textureHandle.IsValid()) continue;
		auto* transform = registry.GetComponent<Transform2DComponent>(entity);
		items_.push_back({ transform, sprite, entity });
	}
}

void SpriteIDPass::Sort() {
	std::sort(items_.begin(), items_.end(), [](const DrawItem& a, const DrawItem& b) {
		if (a.sprite->layer != b.sprite->layer) return a.sprite->layer < b.sprite->layer;
		return a.sprite->orderInLayer < b.sprite->orderInLayer;
		});
}

void SpriteIDPass::MakeLocalQuad(const DrawItem& item, Vector2 out[4]) {
	Vector2 size = { 1.f,1.f };
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

}
}