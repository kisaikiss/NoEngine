#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 吹き出し設定の ImGui エディタシステム。
/// CBSpeechBubbleConfigTag エンティティの SpeechBubbleConfig コンポーネントを直接編集する。
/// 値変更は即時反映（コンポーネントを直接触るため）。
/// </summary>
class SpeechBubbleConfigEditorSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
