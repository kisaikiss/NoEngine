#pragma once
#include "engine/NoEngine.h"

// 話しかけられる側(NPCや看板)に付ける
struct DialogueComponent {
	std::vector<std::string> lines;   // 表示する文字列。改行したい箇所は \n を入れる
	float interactDistance = 3.0f;    // 話しかけられる距離
	bool interactable = true;         // ボタンで話しかけられるか(自動発火専用ならfalse)
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
struct DialogueEndEvent { No::Entity speaker = No::INVALID_ENTITY; };