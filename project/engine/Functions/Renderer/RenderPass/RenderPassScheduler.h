#pragma once
#include "RenderPass.h"
#include "engine/Runtime/Command/GraphicsContext.h"
#include "../RenderContext.h"
#include "../RenderGraph.h"
#include "../RenderResourceManager.h"
namespace NoEngine {
class RenderPassScheduler {
public:
	void AddPass(std::unique_ptr<Render::RenderPass> pass);

	// パイプラインの構築（初期化時、または画面リサイズ・構成変更時に呼ぶ）
	void Compile();

	void Initialize();
	void Render(GraphicsContext& gfx, ECS::Registry& registry);
	void SetRenderContext(RenderContext& renderContext);
	void AddRenderPass(std::unique_ptr<Render::RenderPass>&& pass);

	// リソース登録用（外部からバックバッファなどを登録する）
	RenderGraphRegistry& GetResourceRegistry() { return resourceRegistry_; }

private:
	struct RenderPassNode {
		std::unique_ptr<Render::RenderPass> pass;
		std::vector<std::string> inputs;  // このパスがSRVとして読むリソース
		std::vector<std::string> outputs; // このパスがRTVとして書くリソース
	};

	std::vector<RenderPassNode> nodes_;
	RenderGraphRegistry resourceRegistry_;

	RenderContext renderContext_;

	std::vector<std::unique_ptr<Render::RenderPass>> passes_;
};

void CommonSetupRenderPass(RenderPassScheduler& renderPassScheduler, RenderResourceManager& renderResources);

}


