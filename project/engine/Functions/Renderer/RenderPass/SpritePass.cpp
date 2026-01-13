#include "SpritePass.h"
#include "engine/Math/Types/Calculations/Matrix3x3Calculations.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "../RenderSystem.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"

namespace NoEngine {
namespace Render {

using namespace Component;

namespace {
Matrix4x4 viewProjectionMatrix_;
}

SpritePass::SpritePass() {
	uint32_t indexData[] = { 0, 1, 2, 1, 3, 2 };

	indexBuffer_.Create(L"Sprite Index Buffer", sizeof(indexData), sizeof(uint32_t), indexData);
	indexBufferView_ = indexBuffer_.IndexBufferView();

	viewProjectionMatrix_ = MathCalculations::MakePerspectiveFovMatrix(0.45f, 1.777f, 0.1f, 1000.f);
	viewProjectionMatrix_ = MathCalculations::Inverse(MathCalculations::MakeAffineMatrix({ Vector3::UNIT_SCALE }, { Quaternion::IDENTITY }, { Vector3::ZERO })) * viewProjectionMatrix_;
}

SpritePass::~SpritePass() {}

void SpritePass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	Collect(registry);
	Sort();
	BuildBatches();
	BuildVertexBuffers(gfx);
	Render(gfx);
}

void SpritePass::Collect(ECS::Registry& registry) {
	auto view = registry.View<
		Transform2DComponent,
		SpriteComponent,
		MaterialComponent
	>();

	items_.clear();

	for (auto entity : view) {
		auto* t = registry.GetComponent<Transform2DComponent>(entity);
		auto* s = registry.GetComponent<SpriteComponent>(entity);
		auto* m = registry.GetComponent<MaterialComponent>(entity);

		items_.push_back({ t, s, m, m->pso });
	}
}

void SpritePass::Sort() {
	std::sort(items_.begin(), items_.end(),
		[](const DrawItem& a, const DrawItem& b) {
			return std::tie(a.material->pso,
				a.material->textureHandle,
				a.sprite->layer,
				a.sprite->orderInLayer)
				< std::tie(b.material->pso,
					b.material->textureHandle,
					b.sprite->layer,
					b.sprite->orderInLayer);
		});

}

void SpritePass::BuildBatches() {
	batches_.clear();
	if (items_.empty()) return;

	Batch current{};
	current.pso = items_[0].pso;
	current.texture = items_[0].material->textureHandle;

	size_t batchBegin = 0;

	auto flushBatch = [&](size_t batchEnd) {
		Batch batch = current;
		batch.sprites = std::span<DrawItem>(&items_[batchBegin], batchEnd - batchBegin);
		batches_.push_back(std::move(batch));
		};

	for (size_t i = 1; i < items_.size(); ++i) {
		auto& item = items_[i];
		if (item.pso != current.pso) {
			flushBatch(i);
			batchBegin = i;
			current.pso = item.pso;
			current.texture = item.material->textureHandle;
		}
	}

	flushBatch(items_.size());

}


void SpritePass::BuildVertexBuffers(GraphicsContext& gfx) {
	(void)gfx;
	for (auto& batch : batches_) {
		const size_t spriteCount = batch.sprites.size();
		if (spriteCount == 0) continue;

		const size_t vertexCount = spriteCount * 6;
		tempVertices_.resize(vertexCount);

		size_t cursor = 0;
		for (auto& item : batch.sprites) {
			BuildQuadVertices(
				std::span<SpriteVertex>(tempVertices_.data(), tempVertices_.size()),
				cursor,
				*item.transform,
				*item.sprite,
				*item.material
			);
		}

		batch.vertexCount = static_cast<uint32_t>(vertexCount);
		batch.vertexBuffer.Create(L"Sprite Vertex Buffer", sizeof(SpriteVertex) * static_cast<uint32_t>(tempVertices_.size()), sizeof(SpriteVertex), tempVertices_.data());
	}
}

void SpritePass::Render(GraphicsContext& gfx) {
	if (batches_.empty()) return;

	// 共通の IndexBuffer をセットします。
	// gfx.SetIndexBuffer(indexBufferView_);

	GraphicsPSO* currentPSO = nullptr;
	TextureRef currentTexture;
	
	for (auto& batch : batches_) {
		if (batch.vertexCount == 0) continue;

		if (batch.pso == nullptr) {
			gfx.SetRootSignature(Renderer::gDefaultSpriteRootSignature);
			gfx.SetPipelineState(Renderer::gDefaultSpritePSO);
		} else if (batch.pso != currentPSO) {
			gfx.SetPipelineState(*batch.pso);
			currentPSO = batch.pso;
		}
	
		if (!currentTexture.IsValid() ||
			batch.texture.Get() != currentTexture.Get()) {
			std::unordered_map<std::string, uint32_t>& rootIndex = RootSignatureBuilder::GetRootIndexMap("defaultSpriteRootSignature");
			gfx.SetDynamicConstantBufferView(rootIndex["gCamera"], sizeof(Matrix4x4), &viewProjectionMatrix_);
			gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, batch.texture->GetSRV());
			currentTexture = batch.texture;
		}

		D3D12_VERTEX_BUFFER_VIEW vbv = batch.vertexBuffer.VertexBufferView(0, sizeof(SpriteVertex) * static_cast<uint32_t>(tempVertices_.size()), sizeof(SpriteVertex));
		gfx.SetVertexBuffer(0, vbv);
		gfx.Draw(batch.vertexCount);
	}

}

void SpritePass::BuildQuadVertices(
	std::span<SpriteVertex> dst,
	size_t& cursor,
	const Transform2DComponent& tr,
	const SpriteComponent& sprite,
	const MaterialComponent& mat) {

	// アンカーポイント反映処理
	float left = 0.f - sprite.pivot.x;
	float right = 1.f - sprite.pivot.x;
	float top = 0.f - sprite.pivot.y;
	float bottom = 1.f - sprite.pivot.y;

	Vector2 local[4] = {
		{ left,      bottom   },
		{ left,      top      },
		{ right,     bottom   },
		{ right,     top      },
	};

	// flip
	if (sprite.flipX) {
		for (auto& p : local) p.x = -p.x;
	}
	if (sprite.flipY) {
		for (auto& p : local) p.y = -p.y;
	}

	// Transform2D を適用（2D の position/rotation/scale → 3D 位置へ）
	Matrix3x3 m = tr.MakeAffineMatrix3x3(); // Transform2D に行列生成メソッドがある前提
	Vector2 worldPos[4];
	for (int i = 0; i < 4; ++i) {
		Vector2 p2 = local[i];
		worldPos[i] = MathCalculations::TransformPoint(p2, m);
	}

	// UV（flipX/flipY は UV 側で処理しても良い）
	Rect uv = mat.uv;
	Vector2 uvs[4] = {
		{ uv.x,         uv.y + uv.height },
		{ uv.x + uv.width,  uv.y + uv.height },
		{ uv.x,         uv.y        },
		{ uv.x + uv.width,  uv.y        },
	};

	// 6頂点（0,1,2, 2,1,3）
	auto write = [&](int idx) {
		auto& v = dst[cursor++];
		v.position = Vector4(worldPos[idx].x, worldPos[idx].y, 0.f, 1.f);
		v.texcoord = uvs[idx];
		};

	write(0); write(1); write(2);
	write(2); write(1); write(3);

}

}
}