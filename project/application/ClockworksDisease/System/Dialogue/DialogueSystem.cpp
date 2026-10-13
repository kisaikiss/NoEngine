#include "stdafx.h"
#include "DialogueSystem.h"
#include "../../Component/Dialogue/DialogueComponent.h"
#include "../../Component/Player/PlayerComponent.h"
#include "engine/Runtime/GraphicsCore.h"

#include "engine/Utilities/Conversion/ConvertString.h"

namespace {
// UTF-8文字列の先頭 codepointCount 文字ぶんのバイト長を返す
size_t Utf8ByteLengthOfFirst(const std::string& utf8, size_t codepointCount) {
	size_t bytes = 0;
	size_t count = 0;
	while (bytes < utf8.size() && count < codepointCount) {
		const unsigned char c = static_cast<unsigned char>(utf8[bytes]);
		size_t len = 1;
		if ((c & 0x80) == 0x00) len = 1;
		else if ((c & 0xE0) == 0xC0) len = 2;
		else if ((c & 0xF0) == 0xE0) len = 3;
		else if ((c & 0xF8) == 0xF0) len = 4;
		bytes += len;
		++count;
	}
	return std::min(bytes, utf8.size());
}
}

void DialogueSystem::Update(No::Registry& registry, float deltaTime) {
	if (!initialized_) {
		CreateTextBox(registry);
		initialized_ = true;
	}

	// ウィンドウサイズ変更に追従
	No::Vector2 windowSize = NoEngine::GraphicsCore::GetWindowSize();
	boxWidth_ = windowSize.x * 0.8f;
	registry.GetComponent<No::Transform2DComponent>(boxEntity_)->scale = { boxWidth_, 180.f };
	registry.GetComponent<No::Transform2DComponent>(textEntity_)->translate = { -boxWidth_ * 0.5f + 40.f, -140.f };

	auto startEvents = registry.PollAllEvents<DialogueStartEvent>();

	if (!active_) {
		for (auto& ev : startEvents) {
			if (Begin(registry, ev.speaker)) break;
		}
		return; // 開始したフレームは入力を見ない
	}

	// 文字送り
	if (!IsLineFullyShown()) {
		charTimer_ += deltaTime;
		bool changed = false;
		while (charTimer_ >= secondsPerChar_ && !IsLineFullyShown()) {
			charTimer_ -= secondsPerChar_;
			++charCount_;
			changed = true;
		}
		if (changed) UpdateShownText(registry);
	}

	// ボタン入力
	if (No::InputIsTrigger("Talk")) {
		if (!IsLineFullyShown()) {
			// 途中なら全文を即表示する
			charCount_ = totalChars_;
			UpdateShownText(registry);
		} else {
			Advance(registry);
		}
	}
}

void DialogueSystem::ShowLine(No::Registry& registry) {
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker_);
	totalChars_ = NoEngine::Utf8ToCodepoints(dialogue->lines[lineIndex_]).size();
	charCount_ = 0;
	charTimer_ = 0.0f;

	registry.GetComponent<No::TextComponent>(textEntity_)->isVisible = true;
	registry.GetComponent<No::SpriteComponent>(boxEntity_)->isVisible = true;
	UpdateShownText(registry); // 空文字から始める
}

void DialogueSystem::UpdateShownText(No::Registry& registry) {
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker_);
	if (!dialogue) return;
	const std::string& full = dialogue->lines[lineIndex_];
	registry.GetComponent<No::TextComponent>(textEntity_)->text =
		full.substr(0, Utf8ByteLengthOfFirst(full, charCount_));
}

bool DialogueSystem::Begin(No::Registry& registry, No::Entity speaker) {
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker);
	if (!dialogue || dialogue->lines.empty()) return false;

	active_ = true;
	speaker_ = speaker;
	auto* text = registry.GetComponent<No::TextComponent>(textEntity_);
	text->fontName = dialogue->fontName;
	lineIndex_ = 0;
	for (auto player : registry.View<PlayerComponent>()) {
		if (!registry.Has<DialogueLockTag>(player)) registry.AddComponent<DialogueLockTag>(player);
	}
	ShowLine(registry);
	return true;
}

void DialogueSystem::Advance(No::Registry& registry) {
	++lineIndex_;
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker_); // 相手が消えていてもnullで安全に終われる
	if (!dialogue || lineIndex_ >= dialogue->lines.size()) {
		End(registry);
		return;
	}
	ShowLine(registry);
}

void DialogueSystem::End(No::Registry& registry) {
	registry.GetComponent<No::TextComponent>(textEntity_)->isVisible = false;
	registry.GetComponent<No::SpriteComponent>(boxEntity_)->isVisible = false;
	for (auto player : registry.View<PlayerComponent>()) {
		if (registry.Has<DialogueLockTag>(player)) registry.RemoveComponent<DialogueLockTag>(player);
	}
	registry.EmitEvent(DialogueEndEvent{ speaker_ }); // 終了時の処理(アイテム付与など)はこれを購読する
	active_ = false;
	speaker_ = No::INVALID_ENTITY;
}

void DialogueSystem::CreateTextBox(No::Registry& registry) {
	// 背景
	boxEntity_ = registry.GenerateEntity();
	registry.AddComponent<No::Transform2DComponent>(boxEntity_)->translate = { 0.f, -140.f };
	auto* sprite = registry.AddComponent<No::SpriteComponent>(boxEntity_);
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	sprite->color = No::Color(0.f, 0.f, 0.f, 0.75f);
	sprite->space = NoEngine::SpriteSpace::Screen;
	sprite->anchor = { 0.5f, 1.0f };   // 画面下中央基準
	sprite->layer = 150000;
	sprite->isVisible = false;

	// テキスト
	textEntity_ = registry.GenerateEntity();
	registry.AddComponent<No::Transform2DComponent>(textEntity_);
	auto* text = registry.AddComponent<No::TextComponent>(textEntity_);
	text->fontName = "resources/engine/fonts/PixelMplus12-Regular.ttf";
	text->scale = 0.8f;
	text->hAlign = No::TextComponent::HAlign::kLeft;
	text->vAlign = No::TextComponent::VAlign::kMiddle;
	text->space = NoEngine::SpriteSpace::Screen;
	text->anchor = { 0.5f, 1.0f };
	text->layer = 150001;
	text->isVisible = false;
}

REGISTER_SYSTEM(::DialogueSystem, "DialogueSystem", "UI")