#pragma once
#include "engine/NoEngine.h"

class DialogueSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	bool initialized_ = false;
	bool active_ = false;
	float boxWidth_ = 0.0f;
	size_t lineIndex_ = 0;

	No::Entity boxEntity_ = No::INVALID_ENTITY;
	No::Entity textEntity_ = No::INVALID_ENTITY;
	No::Entity speaker_ = No::INVALID_ENTITY;

	// 文字送り用
	size_t charCount_ = 0;          // 現在表示している文字数(コードポイント単位)
	size_t totalChars_ = 0;         // 現在の行の総文字数
	float charTimer_ = 0.0f;        // 次の1文字までの経過時間
	float secondsPerChar_ = 0.04f;  // 1文字あたりの秒数(小さいほど速い)

	bool IsLineFullyShown() const { return charCount_ >= totalChars_; }
	void UpdateShownText(No::Registry& registry);

	bool Begin(No::Registry& registry, No::Entity speaker);
	void Advance(No::Registry& registry);
	void ShowLine(No::Registry& registry);
	void End(No::Registry& registry);
	void CreateTextBox(No::Registry& registry);
};