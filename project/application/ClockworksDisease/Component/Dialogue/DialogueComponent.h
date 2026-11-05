#pragma once
#include "engine/NoEngine.h"

// 話しかけられる側(NPCや看板)に付ける
struct DialogueComponent {
	std::vector<std::string> lines;   // 表示する文字列。改行したい箇所は \n を入れる
	std::string fontName;
	// 横一列の顔スプライトシート。空欄時はLeimarの既定素材を使う。
	std::string portraitTexture = "resources/game/ClockworksDisease/Texture/Portrait/Leimar.png";
	uint32_t portraitFrameCount = 4;
	// 空欄時はLeimar/01.wav〜03.wav。指定時はこのリストからランダム再生。
	std::vector<std::string> voiceFiles;
	float voiceVolume = 0.35f;
	float interactDistance = 3.0f;    // 話しかけられる距離
	bool interactable = true;         // ボタンで話しかけられるか(自動発火専用ならfalse)
	bool enabled = true;              // 自動発火・グラフを含め、新しい会話の開始を許可するか
};

// 自動発火用(プレイヤーが半径内に入ったら会話を始める)
struct DialogueAutoTriggerComponent {
	float radius = 3.0f;
	bool once = true;
	bool triggered = false;
};

// 会話中にプレイヤーへ付ける。移動/ジャンプ系Systemがこれを見てスキップする
struct DialogueLockTag {};

struct DialogueStartEvent { No::Entity speaker = No::INVALID_ENTITY; };
struct DialogueStopEvent { No::Entity speaker = No::INVALID_ENTITY; };
struct DialogueEndEvent { No::Entity speaker = No::INVALID_ENTITY; };

namespace DialogueGameplayId {
inline constexpr char Play[] = "application.dialogue.play";
inline constexpr char Stop[] = "application.dialogue.stop";
inline constexpr char SetEnabled[] = "application.dialogue.set_enabled";
inline constexpr char Started[] = "application.dialogue.started";
inline constexpr char Finished[] = "application.dialogue.finished";
inline constexpr char Cancelled[] = "application.dialogue.cancelled";
}

// アプリ起動時にエディタとランタイム共通のカタログへ登録する。
void RegisterDialogueGameplay();
