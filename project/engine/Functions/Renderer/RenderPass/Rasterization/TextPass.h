#pragma once
#include "../RenderPass.h"
#include "engine/Functions/ECS/Component/Asset/TextComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Math/Types/Matrix4x4.h"

namespace NoEngine {
namespace Render {

// TextComponentを持つエンティティを文字単位のクアッドに展開して描画するパス。
// SpritePassと対になる構造(Transform2DComponent基準、World/Screen空間、テクスチャ単位でのバッチ描画)。
class TextPass final :
	public RenderPass {
public:
	TextPass();
	~TextPass();
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
	struct DrawItem {
		Component::Transform2DComponent* transform;
		Component::TextComponent* text;
	};
	std::vector<DrawItem> worldItems_;
	std::vector<DrawItem> screenItems_;

	struct TextVertex {
		Math::Vector4 position;
		Math::Vector2 texcoord;
	};
	std::vector<TextVertex> vertices_;
	std::vector<uint16_t> indices_;

	// 1つのTextComponent(=1回のAppendText呼び出し)を1バッチとして描画する。
	// ToDo: 同じフォント・色を使う複数エンティティを1バッチへまとめるとドローコールを減らせる。
	struct Batch {
		TextureRef atlasTexture;
		Math::Color color;
		uint32_t indexStart = 0;
		uint32_t indexCount = 0;
	};
	std::vector<Batch> worldBatches_;
	std::vector<Batch> screenBatches_;

	void CameraUpdate();
	void Collect(ECS::Registry& registry);
	void Sort();
	void GenerateVertices(ECS::Registry& registry);
	void AppendText(const DrawItem& item, bool isScreenSpace, ECS::Registry& registry, std::vector<Batch>& outBatches);
	void Render(GraphicsContext& gfx);
	void DrawBatches(GraphicsContext& gfx, const std::vector<Batch>& batches, std::unordered_map<std::string, uint32_t>& rootIndex);
};

}
}