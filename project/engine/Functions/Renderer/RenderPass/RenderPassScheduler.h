#pragma once
#include "RenderPass.h"
#include "engine/Runtime/Command/GraphicsContext.h"
#include "../RenderContext.h"
#include "../RenderGraph.h"
namespace NoEngine {
class RenderPassScheduler {
public:
	void AddPass(std::unique_ptr<Render::RenderPass> pass, const std::string& editorName = {}, bool enabledByDefault = true, bool editorVisible = true);

	// パイプラインの構築（初期化時、または画面リサイズ・構成変更時に呼ぶ）
	void Compile();

	void Render(GraphicsContext& gfx, ECS::Registry& registry);
	void SetRenderContext(RenderContext& renderContext);
	void AddRenderPass(std::unique_ptr<Render::RenderPass>&& pass);
	void SetScreenDrawBuffer(const std::string& name) { screenDrawBufferName_ = name; }

	void Resize(uint32_t width, uint32_t height);

	ColorBuffer& GetScreenDrawBuffer() { return *resourceRegistry_.GetColorBufferPointer(screenDrawBufferName_); }

	// リソース登録用（外部からバックバッファなどを登録する）
	RenderGraphRegistry& GetResourceRegistry() { return resourceRegistry_; }

private:
	struct RenderPassNode {
		std::unique_ptr<Render::RenderPass> pass;
		std::string editorName;
		bool enabledByDefault = true;
		bool editorVisible = true;
		std::vector<std::string> inputs;  // このパスがSRVとして読むリソース
		std::vector<RenderGraphBuilder::OutputDesc> outputs; // このパスがRTVとして書くリソース
		std::vector<std::string> uavOutputs; // このパスがUAVとして書くリソース
		RenderGraphBuilder::OutputDesc depthOutput;
		bool hasDepthOutput = false;
	};

	std::vector<RenderPassNode> nodes_;
	std::vector<std::string> activePassNames_;
	std::unordered_map<std::string, nlohmann::json> lastSavedSettings_;
	std::unordered_map<std::string, nlohmann::json> defaultPassSettings_;
	std::unordered_set<std::string> invalidPassWarnings_;
	std::string loadedSceneName_;
	std::filesystem::file_time_type loadedPassConfigWriteTime_{};
	bool passConfigLoaded_ = false;
	void LoadPassConfiguration(ECS::Registry& registry);
	void SavePassConfiguration(ECS::Registry& registry);
	void DrawPassEditor(ECS::Registry& registry);
	void RefreshPassNode(RenderPassNode& node);
	RenderGraphRegistry resourceRegistry_;

	RenderContext renderContext_;

	std::string screenDrawBufferName_ = "MainColor";

	// クリックした場所のObjectIDテクスチャの色
	ReadbackBuffer idReadbackBuffer_;

	// クリックを離した場所のWorldPositionテクスチャの色
	ReadbackBuffer positionReadbackBuffer_;

	std::vector<std::unique_ptr<Render::RenderPass>> passes_;
};

void CommonSetupRenderPass(RenderPassScheduler& renderPassScheduler);
void CommonSetupDebugRenderPass(RenderPassScheduler& renderPassScheduler);

}


