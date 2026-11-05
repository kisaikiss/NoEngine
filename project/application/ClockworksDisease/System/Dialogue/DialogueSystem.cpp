#include "stdafx.h"
#include "DialogueSystem.h"
#include "../../Component/Dialogue/DialogueComponent.h"
#include "../../Component/Player/PlayerComponent.h"
#include "engine/Runtime/GraphicsCore.h"

#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Assets/Audio/Audio.h"
#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"

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
// 基準解像度(この解像度で見た目をデザインする)
constexpr float kBaseWidth = 1280.f;
constexpr float kBaseHeight = 720.f;

// 基準解像度でのレイアウト値
constexpr float kBaseBoxHeight = 180.f;
constexpr float kBaseBoxOffsetY = -140.f;
constexpr float kBaseTextPaddingX = 40.f;
constexpr float kBaseTextScale = 0.55f;
constexpr float kBasePortraitSize = 128.0f;

std::mt19937& VoiceRandom() {
	static std::mt19937 generator{ std::random_device{}() };
	REGISTER_PLAY_MODE_STATIC(generator);
	return generator;
}

std::unordered_map<std::string, bool>& LoadedDialogueVoices() {
	static std::unordered_map<std::string, bool> loaded;
	REGISTER_PLAY_MODE_STATIC(loaded);
	return loaded;
}

}

void DialogueSystem::Update(No::Registry& registry, float deltaTime) {
	if (!initialized_) {
		CreateTextBox(registry);
		initialized_ = true;
	}

	// ウィンドウサイズ変更に追従
	UpdateLayout(registry);

	auto startEvents = registry.PollAllEvents<DialogueStartEvent>();
	bool began = false;
	if (!active_) {
		for (auto& ev : startEvents) {
			if (Begin(registry, ev.speaker)) {
				began = true;
				break;
			}
		}
	}
	// 同じフレームの開始要求にもStopを適用し、他の話者へのStopは無視する。
	for (const auto& ev : registry.PollAllEvents<DialogueStopEvent>()) {
		if (active_ && ev.speaker == speaker_) End(registry, true);
	}
	if (!active_) return;
	// 会話中に対象や本文が削除された場合もロックとUIを確実に解除する。
	const auto* currentDialogue = registry.GetComponent<DialogueComponent>(speaker_);
	if (!currentDialogue || lineIndex_ >= currentDialogue->lines.size()) {
		End(registry, true);
		return;
	}
	if (began) return; // 開始したフレームは入力を見ない

	// 文字送り
	if (!IsLineFullyShown()) {
		charTimer_ += deltaTime;
		bool changed = false;
		size_t previousCharCount = charCount_;
		while (charTimer_ >= secondsPerChar_ && !IsLineFullyShown()) {
			charTimer_ -= secondsPerChar_;
			++charCount_;
			changed = true;
		}
		if (changed) {
			UpdateShownText(registry);
			// 表示が進んだ文字数ぶん顔を動かし、音声は新しく表示された文字ごとに鳴らす。
			auto* dialogue = registry.GetComponent<DialogueComponent>(speaker_);
			const auto codepoints = NoEngine::Utf8ToCodepoints(dialogue->lines[lineIndex_]);
			for (size_t i = previousCharCount; i < charCount_; ++i) {
				UpdatePortraitFrame(registry, ++portraitFrame_ % std::max<uint32_t>(1, dialogue->portraitFrameCount));
				PlayVoiceForCharacter(registry, codepoints[i]);
			}
			// 行末まで表示したら、次の入力待ち中は先頭フレームにする。
			if (IsLineFullyShown()) {
				portraitFrame_ = 0;
				UpdatePortraitFrame(registry, 0);
			}
		}
	}

	// ボタン入力
	if (No::InputIsTrigger("Talk")) {
		if (!IsLineFullyShown()) {
			// 途中なら全文を即表示する
			charCount_ = totalChars_;
			UpdateShownText(registry);
			// スキップ表示では音声やアニメーションを連続再生しない。
			UpdatePortraitFrame(registry, 0);
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
	registry.GetComponent<No::SpriteComponent>(portraitEntity_)->isVisible = true;
	portraitFrame_ = 0;
	UpdatePortraitFrame(registry, 0);
	UpdateShownText(registry); // 空文字から始める
}

void DialogueSystem::UpdateShownText(No::Registry& registry) {
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker_);
	if (!dialogue) return;
	const std::string& full = dialogue->lines[lineIndex_];
	registry.GetComponent<No::TextComponent>(textEntity_)->text =
		full.substr(0, Utf8ByteLengthOfFirst(full, charCount_));
}

void DialogueSystem::UpdatePortraitFrame(No::Registry& registry, size_t frame) {
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker_);
	auto* sprite = registry.GetComponent<No::SpriteComponent>(portraitEntity_);
	if (!dialogue || !sprite) return;
	const uint32_t count = std::max<uint32_t>(1, dialogue->portraitFrameCount);
	frame %= count;
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture(dialogue->portraitTexture);
	sprite->uv = { static_cast<float>(frame) / count, 0.0f, 1.0f / count, 1.0f };
}

void DialogueSystem::PlayVoiceForCharacter(No::Registry& registry, char32_t codepoint) {
	if (codepoint == U' ' || codepoint == U'\n' || codepoint == U'\r' || codepoint == U'\t') return;
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker_);
	if (!dialogue) return;
	std::vector<std::string> paths = dialogue->voiceFiles;
	if (paths.empty()) {
		for (int i = 1; i <= 3; ++i) {
			char filename[8];
			sprintf_s(filename, "%02d.wav", i);
			paths.push_back(std::string("resources/game/ClockworksDisease/Sounds/Voice/Leimar/") + filename);
		}
	}
	if (paths.empty()) return;
	std::uniform_int_distribution<size_t> pick(0, paths.size() - 1);
	size_t index = pick(VoiceRandom());
	if (paths.size() > 1 && index == lastVoiceIndex_) index = (index + 1 + pick(VoiceRandom()) % (paths.size() - 1)) % paths.size();
	lastVoiceIndex_ = index;
	const std::string& path = paths[index];
	auto& loaded = LoadedDialogueVoices();
	if (!loaded[path]) {
		NoEngine::Asset::SoundLoad(NoEngine::ConvertString(path), path);
		loaded[path] = true;
	}
	NoEngine::Asset::SoundEffectPlay(path, std::clamp(dialogue->voiceVolume, 0.0f, 1.0f));
}

void DialogueSystem::UpdateLayout(No::Registry& registry) {
	No::Vector2 windowSize = NoEngine::GraphicsCore::GetWindowSize();

	// 幅・高さそれぞれの比率のうち小さい方を採用する。
	// こうすると縦長・横長どちらのウィンドウでも文字がボックスに収まる
	const float scale = std::min(windowSize.x / kBaseWidth, windowSize.y / kBaseHeight);

	const float boxWidth = windowSize.x * 0.8f;
	const float boxHeight = kBaseBoxHeight * scale;
	const float boxOffsetY = kBaseBoxOffsetY * scale;

	auto* boxT = registry.GetComponent<No::Transform2DComponent>(boxEntity_);
	boxT->scale = { boxWidth, boxHeight };
	boxT->translate = { 0.f, boxOffsetY };

	auto* textT = registry.GetComponent<No::Transform2DComponent>(textEntity_);
	textT->translate = { -boxWidth * 0.5f + (kBasePortraitSize + kBaseTextPaddingX) * scale, boxOffsetY };
	auto* portraitT = registry.GetComponent<No::Transform2DComponent>(portraitEntity_);
	portraitT->scale = { kBasePortraitSize * scale, kBasePortraitSize * scale };
	portraitT->translate = { -boxWidth * 0.5f + kBasePortraitSize * 0.65f * scale, boxOffsetY + kBasePortraitSize * 0.12f * scale };

	registry.GetComponent<No::TextComponent>(textEntity_)->scale = kBaseTextScale * scale;

}

bool DialogueSystem::Begin(No::Registry& registry, No::Entity speaker) {
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker);
	if (!dialogue || !dialogue->enabled || dialogue->lines.empty()) return false;

	active_ = true;
	speaker_ = speaker;
	auto* text = registry.GetComponent<No::TextComponent>(textEntity_);
	text->fontName = dialogue->fontName;
	lastVoiceIndex_ = static_cast<size_t>(-1);
	lineIndex_ = 0;
	for (auto player : registry.View<PlayerComponent>()) {
		if (!registry.Has<DialogueLockTag>(player)) registry.AddComponent<DialogueLockTag>(player);
	}
	ShowLine(registry);
	NoEngine::Gameplay::EmitGameplayEvent(registry, speaker_, DialogueGameplayId::Started);
	return true;
}

void DialogueSystem::Advance(No::Registry& registry) {
	++lineIndex_;
	auto* dialogue = registry.GetComponent<DialogueComponent>(speaker_); // 相手が消えていてもnullで安全に終われる
	if (!dialogue) {
		End(registry, true);
		return;
	}
	if (lineIndex_ >= dialogue->lines.size()) {
		End(registry);
		return;
	}
	ShowLine(registry);
}

void DialogueSystem::End(No::Registry& registry, bool cancelled) {
	if (!active_) return;
	registry.GetComponent<No::TextComponent>(textEntity_)->isVisible = false;
	registry.GetComponent<No::SpriteComponent>(boxEntity_)->isVisible = false;
	registry.GetComponent<No::SpriteComponent>(portraitEntity_)->isVisible = false;
	UpdatePortraitFrame(registry, 0);
	portraitFrame_ = 0;
	for (auto player : registry.View<PlayerComponent>()) {
		if (registry.Has<DialogueLockTag>(player)) registry.RemoveComponent<DialogueLockTag>(player);
	}
	// 既存の報酬・チュートリアル処理には正常終了だけを通知する。
	if (!cancelled) registry.EmitEvent(DialogueEndEvent{ speaker_ });
	NoEngine::Gameplay::EmitGameplayEvent(registry, speaker_,
		cancelled ? DialogueGameplayId::Cancelled : DialogueGameplayId::Finished);
	active_ = false;
	speaker_ = No::INVALID_ENTITY;
}

void DialogueSystem::CreateTextBox(No::Registry& registry) {
	// 背景
	boxEntity_ = registry.GenerateEntity();
	registry.AddComponent<No::Transform2DComponent>(boxEntity_)->translate = { 0.f, kBaseBoxOffsetY };
	auto* sprite = registry.AddComponent<No::SpriteComponent>(boxEntity_);
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	sprite->color = No::Color(0.f, 0.f, 0.f, 0.75f);
	sprite->space = NoEngine::SpriteSpace::Screen;
	sprite->anchor = { 0.5f, 1.0f };   // 画面下中央基準
	sprite->layer = 150000;
	sprite->isVisible = false;

	// 話者の顔アイコン。4コマを横に並べたシートを想定。
	portraitEntity_ = registry.GenerateEntity();
	registry.AddComponent<No::Transform2DComponent>(portraitEntity_);
	auto* portrait = registry.AddComponent<No::SpriteComponent>(portraitEntity_);
	portrait->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/ClockworksDisease/Texture/Portrait/Leimar.png");
	portrait->uv = { 0.0f, 0.0f, 0.25f, 1.0f };
	portrait->space = NoEngine::SpriteSpace::Screen;
	portrait->anchor = { 0.5f, 1.0f };
	portrait->layer = 150001;
	portrait->isVisible = false;

	// テキスト
	textEntity_ = registry.GenerateEntity();
	registry.AddComponent<No::Transform2DComponent>(textEntity_);
	auto* text = registry.AddComponent<No::TextComponent>(textEntity_);
	text->fontName = "resources/engine/fonts/PixelMplus12-Regular.ttf";
	text->scale = kBaseTextScale;
	text->hAlign = No::TextComponent::HAlign::kLeft;
	text->vAlign = No::TextComponent::VAlign::kMiddle;
	text->space = NoEngine::SpriteSpace::Screen;
	text->anchor = { 0.5f, 1.0f };
	text->layer = 150002;
	text->isVisible = false;
}

REGISTER_SYSTEM(::DialogueSystem, "DialogueSystem", "UI")
