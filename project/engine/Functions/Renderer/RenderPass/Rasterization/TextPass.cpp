#include "TextPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {

using namespace Component;
using namespace Math;
namespace {
Matrix4x4 sCameraMatrix;
}

TextPass::TextPass() {
	auto size = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	sCameraMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(size.clientWidth), static_cast<float>(size.clientHeight), 0.1f, 100.f);
}

TextPass::~TextPass() {}

void TextPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	(void)resourceRegistry;
	CameraUpdate();
	Collect(registry);
	Sort();
	GenerateVertices(registry);
	Render(gfx);
}

void TextPass::CameraUpdate() {
	auto* camera = GetTargetCamera2D();
	if (camera) {
		sCameraMatrix = camera->viewProjection;
	} else {
		Vector2 windowSize = GraphicsCore::GetWindowSize();
		sCameraMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(windowSize.x), static_cast<float>(windowSize.y), 0.1f, 100.f);
	}
}

void TextPass::Collect(ECS::Registry& registry) {
	auto view = registry.View<Transform2DComponent, TextComponent>();

	worldItems_.clear();
	screenItems_.clear();

	for (auto entity : view) {
		auto* text = registry.GetComponent<TextComponent>(entity);
		if (!text->isVisible || text->text.empty() || !text->fontHandle.IsValid()) continue;
		auto* transform = registry.GetComponent<Transform2DComponent>(entity);

		if (text->space == SpriteSpace::Screen) {
			screenItems_.push_back({ transform, text });
		} else {
			worldItems_.push_back({ transform, text });
		}
	}
}

void TextPass::Sort() {
	// テクスチャでのソートはSpritePassと違い行わない(フォントごとに個別バッチにしているため)。
	// layer/orderInLayerだけを基準に、ブレンド順(ペインターズアルゴリズム)を制御する。
	auto comparator = [](const DrawItem& a, const DrawItem& b) {
		if (a.text->layer != b.text->layer) return a.text->layer < b.text->layer;
		return a.text->orderInLayer < b.text->orderInLayer;
		};
	std::sort(worldItems_.begin(), worldItems_.end(), comparator);
	std::sort(screenItems_.begin(), screenItems_.end(), comparator);
}

void TextPass::AppendText(const DrawItem& item, bool isScreenSpace, ECS::Registry& registry, std::vector<Batch>& outBatches) {
	auto* t = item.transform;
	auto* text = item.text;

	const Font* font = text->fontHandle.Get();
	if (!font) return;

	// '\n'で行分割する(hAlignの計算に各行の幅が必要なため)
	std::vector<std::string> lines;
	{
		std::string current;
		for (char c : text->text) {
			if (c == '\n') { lines.push_back(current); current.clear(); } else current.push_back(c);
		}
		lines.push_back(current);
	}

	std::vector<float> lineWidths;
	lineWidths.reserve(lines.size());
	for (auto& line : lines) {
		float width = 0.f;
		for (char c : line) {
			const GlyphInfo* g = font->GetGlyph(static_cast<char32_t>(static_cast<unsigned char>(c)));
			if (!g) continue;
			width += g->advance + text->letterSpacing;
		}
		lineWidths.push_back(width);
	}

	const float ascent = font->GetAscent();
	const float lineHeight = font->GetLineHeight();
	const float blockHeight = lineHeight * static_cast<float>(lines.size());

	float vOffset = 0.f;
	switch (text->vAlign) {
	case TextComponent::VAlign::kTop:    vOffset = ascent; break;
	case TextComponent::VAlign::kMiddle: vOffset = ascent - blockHeight * 0.5f; break;
	case TextComponent::VAlign::kBaseline:
	default: vOffset = 0.f; break;
	}

	Matrix4x4 mat = t->MakeAffineMatrix4x4(registry);
	Vector2 windowSize = GraphicsCore::GetWindowSize();

	Batch batch;
	batch.atlasTexture = font->GetAtlasTexture();
	batch.color = text->color;
	batch.indexStart = static_cast<uint32_t>(indices_.size());

	for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
		const std::string& line = lines[lineIndex];

		float hOffset = 0.f;
		switch (text->hAlign) {
		case TextComponent::HAlign::kCenter: hOffset = -lineWidths[lineIndex] * 0.5f; break;
		case TextComponent::HAlign::kRight:  hOffset = -lineWidths[lineIndex]; break;
		case TextComponent::HAlign::kLeft:
		default: hOffset = 0.f; break;
		}

		float cursorX = hOffset;
		const float cursorY = vOffset + lineHeight * static_cast<float>(lineIndex);

		for (char c : line) {
			const GlyphInfo* g = font->GetGlyph(static_cast<char32_t>(static_cast<unsigned char>(c)));
			if (!g) continue;

			if (g->size.x > 0.f && g->size.y > 0.f) {
				const float localLeft = (cursorX + g->bearing.x) * text->scale;
				const float localTop = (cursorY + g->bearing.y) * text->scale;
				const float localRight = localLeft + g->size.x * text->scale;
				const float localBottom = localTop + g->size.y * text->scale;

				Vector3 local[4] = {
					{ localLeft,  localTop,    0.f },
					{ localRight, localTop,    0.f },
					{ localLeft,  localBottom, 0.f },
					{ localRight, localBottom, 0.f },
				};

				Vector2 uvs[4] = {
					{ g->uv.x,               g->uv.y },
					{ g->uv.x + g->uv.width, g->uv.y },
					{ g->uv.x,               g->uv.y + g->uv.height },
					{ g->uv.x + g->uv.width, g->uv.y + g->uv.height },
				};

				uint16_t indexOffset = static_cast<uint16_t>(vertices_.size());
				for (int i = 0; i < 4; ++i) {
					Vector3 world = MathCalculations::Transform(local[i], mat);
					if (isScreenSpace) {
						world.x += text->anchor.x * windowSize.x;
						world.y += text->anchor.y * windowSize.y;
					}
					vertices_.push_back({ { world.x, world.y, 0.f, 1.f }, uvs[i] });
				}

				indices_.push_back(indexOffset + 0);
				indices_.push_back(indexOffset + 1);
				indices_.push_back(indexOffset + 2);
				indices_.push_back(indexOffset + 1);
				indices_.push_back(indexOffset + 3);
				indices_.push_back(indexOffset + 2);
			}

			cursorX += g->advance + text->letterSpacing;
		}
	}

	batch.indexCount = static_cast<uint32_t>(indices_.size()) - batch.indexStart;
	if (batch.indexCount > 0) {
		outBatches.push_back(batch);
	}
}

void TextPass::GenerateVertices(ECS::Registry& registry) {
	vertices_.clear();
	indices_.clear();
	worldBatches_.clear();
	screenBatches_.clear();

	for (auto& item : worldItems_) {
		AppendText(item, false, registry, worldBatches_);
	}
	for (auto& item : screenItems_) {
		AppendText(item, true, registry, screenBatches_);
	}
}

void TextPass::Render(GraphicsContext& gfx) {
	if (vertices_.empty()) return;
	auto* renderCtx = GetRenderContext();
	std::unordered_map<std::string, uint32_t>& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Text PSO");
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : Text PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : Text PSO"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicVB(0, vertices_.size(), sizeof(TextVertex), vertices_.data());
	gfx.SetDynamicIB(indices_.size(), indices_.data());

	// --- World空間: カメラ行列を使用 ---
	if (!worldBatches_.empty()) {
		gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &sCameraMatrix);
		DrawBatches(gfx, worldBatches_, rootIndex);
	}

	// --- Screen空間(UI): 常に現在のウィンドウサイズ基準、カメラ無視 ---
	if (!screenBatches_.empty()) {
		Vector2 windowSize = GraphicsCore::GetWindowSize();
		Matrix4x4 screenMatrix = MathCalculations::MakeOrthographicMatrix(
			0.f, 0.f, windowSize.x, windowSize.y, 0.1f, 100.f);
		gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &screenMatrix);
		DrawBatches(gfx, screenBatches_, rootIndex);
	}
}

void TextPass::DrawBatches(GraphicsContext& gfx, const std::vector<Batch>& batches, std::unordered_map<std::string, uint32_t>& rootIndex) {
	for (auto& batch : batches) {
		_declspec(align(16)) struct {
			Color color;
		} materialConstants;
		materialConstants.color = batch.color;
		gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(materialConstants), &materialConstants);

		gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, batch.atlasTexture.GetSRV());
		gfx.DrawIndexedInstanced(batch.indexCount, 1, batch.indexStart, 0, 0);
	}
}

}
}