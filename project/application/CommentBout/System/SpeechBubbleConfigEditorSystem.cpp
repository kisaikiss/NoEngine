#include "stdafx.h"
#include "SpeechBubbleConfigEditorSystem.h"
#include "application/CommentBout/Data/SpeechBubbleConfig.h"
#include "application/CommentBout/Data/SpeechBubbleDataIO.h"
#include "application/CommentBout/GameTag.h"

void SpeechBubbleConfigEditorSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);


#ifdef USE_IMGUI
	SpeechBubbleConfig* cfg = nullptr;
	auto view = registry.View<CBSpeechBubbleConfigTag, SpeechBubbleConfig>();
	for (auto e : view) {
		cfg = registry.GetComponent<SpeechBubbleConfig>(e);
		if (cfg) break;
	}
	if (!cfg) return;

	ImGui::Begin("吹き出し設定");

	ImGui::SeparatorText("距離閾値（カメラからの3D距離）");
	ImGui::DragFloat("大サイズ 最大距離",   &cfg->largeMaxDistance,  0.1f, 0.1f, 100.f);
	ImGui::DragFloat("中サイズ 最大距離",   &cfg->mediumMaxDistance, 0.1f, 0.1f, 100.f);
	if (cfg->largeMaxDistance >= cfg->mediumMaxDistance)
		cfg->largeMaxDistance = cfg->mediumMaxDistance - 0.1f;
	ImGui::TextDisabled("中距離 〜 ∞ → 小サイズ");

	ImGui::Separator();

	auto editSize = [](const char* label, SpeechBubbleSizeConfig& s) {
		if (!ImGui::TreeNode(label)) return;
		ImGui::DragFloat2("スプライトサイズ", &s.spriteSize.x,  1.f, 4.f, 512.f);
		ImGui::DragInt  ("ダメージ量",        &s.attackPower,   1,   1,   100);
		ImGui::DragFloat("飛翔時間(秒)",      &s.duration,      0.01f, 0.1f, 5.f);
		ImGui::TreePop();
	};

	editSize("大 (POW.png)",   cfg->sizeLarge);
	editSize("中 (BOOM.png)",  cfg->sizeMedium);
	editSize("小 (OH.png)",    cfg->sizeSmall);

	ImGui::Separator();
	if (ImGui::Button("JSONへ保存")) {
		SpeechBubbleDataIO::Save(*cfg);
	}

	ImGui::End();
#else
	static_cast<void>(registry);
#endif
}
